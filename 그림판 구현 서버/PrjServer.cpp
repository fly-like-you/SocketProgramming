#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기

#include "Common.h"
#include "resource.h"
#include "server.h" // 메시지 구조체 및 상수 정의 
#include "serverFunction.h" // 스레드 관련 전역변수 및 함수 정의
#include "udpServerFunc.h"
// 콜백 프로시저
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

// UDP, TCP 스레드 분리 이벤트
// 수신 이벤트와 송신 이벤트로 분리 시키기
HANDLE hRecvEvent;
HANDLE hSendEvent;
char* translationBuffer;
HEAD_MSG    g_head_msg;         // 박준호 추가 고정 크기 메시지  헤더

DWORD WINAPI DialogThread(LPVOID arg) {
	HINSTANCE hInstance = (HINSTANCE)arg;
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
	return 0;
}

DWORD WINAPI ClientRecvUdpThread(LPVOID arg) {
	// UDP 수신
	//	 UDP는 바로 받으면 되기 때문에 for문 하나는 필요없
	// 
	// 데이터 통신에 사용할 변수
	int retval;
	struct sockaddr_in clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];

	// 클라이언트와 데이터 통신
	while (1) {
		char* buf;
		struct sockaddr_in udpAddr;

		addrlen = sizeof(udpAddr);

		//고정길이 수신
		retval = recvfrom(g_udpsock, (char*)&g_head_msg, sizeof(HEAD_MSG), 0, (struct sockaddr*)&udpAddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			RemoveUdpSocketInfo(&udpAddr);
			continue;
		}
		buf = (char*)malloc(g_head_msg.length * sizeof(char));

		translationBuffer = (char*)malloc(g_head_msg.length * sizeof(char)); // 문제점 버퍼가 공유변수이기 때문에 불안정할수도있음

		//가변길이 수신
		retval = recvfrom(g_udpsock, (char*)buf, g_head_msg.length, 0, (struct sockaddr*)&udpAddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			RemoveUdpSocketInfo(&udpAddr);
			continue;
		}

		// 소켓 정보 구조체에 추가
		// 일단 한번이라도 등록이 되어야하기 때문에 메시지를 보내야만 보임
		if (!compareUdpSocketArray(&udpAddr)) { // 구조체를 뒤져서 없으면
			AddUdpSocketInfo(udpAddr);			// 구조체에 추가하기

		}
		memcpy(translationBuffer, buf, g_head_msg.length);


		free(buf);

		SetEvent(hRecvEvent); // 상대 이벤트 키기
	}



	
	return 0;
}
DWORD WINAPI ClientRecvTcpThread(LPVOID arg) {
	// 첫 접속 시에는 이름과 펜 색깔을 정해줘야 함
	//블랙리스트 처리
	// 공지사항 전파
	fd_set rset;
	fd_set wset;
	SOCKET client_sock;
	int addrlen;
	// 데이터 통신에 사용할 변수(IPv4)


	while (1) {
		//retval = WaitForSingleObject(hSendEvent, INFINITE);
		//if (retval != WAIT_OBJECT_0) break;
		// 박준호 추가
		struct sockaddr_in clientaddr4;
		struct sockaddr_in udpAddr;
		WELCOME_MSG welcome_msg;

		// UDP 변수
		addrlen = sizeof(clientaddr4);

		// 소켓 셋 초기화 후 udp, tcp소켓 삽입
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_SET(g_listensock, &rset);
		//FD_SET(g_udpsock, &rset); // UDP 넣기

		for (int i = 0; i < nTotalSockets; i++) {
			if( SocketInfoArray[i]->recvbytes > SocketInfoArray[i]->sendbytes)
				FD_SET(SocketInfoArray[i]->sock, &wset);

			else
				FD_SET(SocketInfoArray[i]->sock, &rset);	
		}

		// select()
		retval = select(0, &rset, &wset, NULL, NULL);



		if (retval == SOCKET_ERROR) err_quit("select()");

		// 소켓 셋 검사(1): 클라이언트 접속 수용
		if (FD_ISSET(g_listensock, &rset)) { // TCP/IPv4
			addrlen = sizeof(clientaddr4);
			client_sock = accept(g_listensock,
				(struct sockaddr*)&clientaddr4, &addrlen);
			if (client_sock == INVALID_SOCKET) {
				err_display("accept()");
				break;
			}
			else {
				// 최초로 리시브 받아줌
				recv(client_sock, (char*)&welcome_msg, sizeof(WELCOME_MSG), MSG_WAITALL);
				if (!AddSocketInfo(client_sock, false, welcome_msg.nickname))
					closesocket(client_sock);
			}
		}

		// 소켓 셋 검사(2): 데이터 통신
		for (int i = 0; i < nTotalSockets; i++) {
			SOCKETINFO* ptr = SocketInfoArray[i];
			if (FD_ISSET(ptr->sock, &rset)) {

				//  박준호 추가 고정 크기 데이터 받기
				retval = recv(ptr->sock, (char*)&g_head_msg, sizeof(HEAD_MSG), MSG_WAITALL);
				if (retval == 0 || retval == SOCKET_ERROR) {
					RemoveSocketInfo(i);
					continue;
				}
				ptr->buf = (char*)malloc(g_head_msg.length * sizeof(char));
				translationBuffer = (char*)malloc(g_head_msg.length * sizeof(char));
				retval = recv(ptr->sock, (char*)ptr->buf, g_head_msg.length, MSG_WAITALL);

				memcpy(translationBuffer, ptr->buf, g_head_msg.length);
				if (retval == 0 || retval == SOCKET_ERROR) {
					RemoveSocketInfo(i);
					continue;
				}
				
				free(ptr->buf);

				SetEvent(hRecvEvent); // 상대 이벤트 키기
				// 이거 TCP랑 UDP랑 같이 켜지면 어케댐?
			}	
		} /* end of for */


		
	} /* end of while (1) */
	return 0;
}
DWORD WINAPI ClientSendThread(LPVOID arg) {

	while (1) {
		retval = WaitForSingleObject(hRecvEvent, INFINITE); // 읽기 완료 대기

		if (retval != WAIT_OBJECT_0) break;
		// 현재 접속한 모든 클라이언트에 데이터 전송
		// TCP 전송

		struct sockaddr_in clientaddr4;
		struct sockaddr_in udpAddr;
		for (int j = 0; j < nTotalSockets; j++) {
			SOCKETINFO* ptr2 = SocketInfoArray[j];

			retval = send(ptr2->sock, (char*)&g_head_msg, sizeof(HEAD_MSG), 0); // 받은 데이터를 그대로 돌려주기 때문에 head_msg 재사용 서버에서 처리를 하는 경우에는 바껴야댐
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				RemoveSocketInfo(j);
				--j; // 루프 인덱스 보정
				continue;
			}
			// 헤더에서 받은 길이는 보내는 만큼의 크기
			retval = send(ptr2->sock, (char*)translationBuffer, g_head_msg.length, 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				RemoveSocketInfo(j);
				--j; // 루프 인덱스 보정
				continue;
			}
		}

		// UDP 전송
		for (int k = 0; k < nTotalUdpSockets; k++) {
			udpSocketInfo* ptrUdp = udpSocketInfoArray[k];
			//고정길이 전송
			retval = sendto(g_udpsock, (char*)&g_head_msg, sizeof(HEAD_MSG), 0, (struct sockaddr*)&ptrUdp->sockaddr, sizeof(ptrUdp->sockaddr));
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				RemoveUdpSocketInfo(&ptrUdp->sockaddr);
				--k; // 루프 인덱스 보정
				continue;
			}

			// 가변길이 전송
			retval = sendto(g_udpsock, (char*)translationBuffer, g_head_msg.length, 0, (struct sockaddr*)&ptrUdp->sockaddr, sizeof(ptrUdp->sockaddr));
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				RemoveUdpSocketInfo(&ptrUdp->sockaddr);
				--k; // 루프 인덱스 보정
				continue;
			}

		}
		free(translationBuffer);

		//SetEvent(hSendEvent);
		ResetEvent(hRecvEvent);
	}
	return 0;
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	g_chatmsg.type = TYPE_CHAT;

	SetSock(g_listensock, SOCK_STREAM);
	SetSock(g_udpsock, SOCK_DGRAM);

	/* 이벤트 객체 처리 */
	hRecvEvent = CreateEvent(NULL, TRUE, FALSE, NULL);  // 클라이언트로 부터 데이터 받기 완료
	//hSendEvent = CreateEvent(NULL, TRUE, FALSE, NULL);  // 클라이언트로 부터 데이터 전송 완료

	HANDLE hClientRecvUdpThread = CreateThread(NULL, 0, ClientRecvUdpThread, NULL, 0, NULL);
	HANDLE hClientRecvTcpThread = CreateThread(NULL, 0, ClientRecvTcpThread, NULL, 0, NULL);
	HANDLE hClientWriteThread = CreateThread(NULL, 0, ClientSendThread, NULL, 0, NULL);

	//SetEvent(hSendEvent);

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
	
	closesocket(g_listensock);
	closesocket(g_udpsock);

	WSACleanup();
	return 0;
}

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {


	/* 각종 핸들*/
	static HWND heditIPaddr;
	//static HINSTANCE  hInst; // 전역 변수에도 저장
	static HWND hBtnblackAppend; // 전역 변수에도 저장
	static HWND hBtnblackRemove; // 전역 변수에도 저장
	static HWND hBtnfilterAppend; // 전역 변수에도 저장
	static HWND hBtnfilterRemove; // 전역 변수에도 저장
	static HWND hBtnReset;
	static HWND heditFilterMsg;
	static HWND heditStatus;


	/* paint를 위한 변수들 */
	HDC hdc;
	PAINTSTRUCT ps;
	const char* clientlist = "클라이언트 목록";
	const char* blacklist = "블랙리스트 목록";
	const char* filterlist = "금지 단어";

	/* 클라이언트, 블랙리스트 목록들 위한 변수들*/
	int i;
	int row;
	int column;
	static HWND CList; //클라이언트 목록 리스트
	static HWND BList; //블랙리스트 목록
	LVCOLUMN col1; //Clinetlist 컬럼
	LVCOLUMN col2; //Blacklist 컬럼
	LVITEM clientLI;
	//LVITEM blackLI;


	int CLcolwidth[3] = { 50, 230, 50 };
	LPWSTR CLcolname[3] = { (LPWSTR)TEXT("id"), (LPWSTR)TEXT("IP") , (LPWSTR)TEXT("색") };

	int BLcolwidth[2] = { 50,  348 };
	LPWSTR BLcolname[2] = { (LPWSTR)TEXT("id"), (LPWSTR)TEXT("IP") };

	switch (uMsg) {
	case WM_INITDIALOG:

		heditIPaddr = GetDlgItem(hDlg, BLACKADDR);
		hBtnblackAppend = GetDlgItem(hDlg, blackAppend);
		g_hBtnblackAppend = hBtnblackAppend;
		hBtnblackRemove = GetDlgItem(hDlg, blackRemv);;
		g_hBtnblackRemove = hBtnblackRemove;
		hBtnfilterAppend = GetDlgItem(hDlg, filterAppend);
		g_hBtnfilterAppend = hBtnfilterAppend;
		hBtnfilterRemove = GetDlgItem(hDlg, filterRemove);
		g_hBtnfilterRemove = hBtnfilterRemove;
		heditFilterMsg = GetDlgItem(hDlg, filterWord);
		heditStatus = GetDlgItem(hDlg, filterList);
		g_hEditStatus = heditStatus;
		hBtnReset = GetDlgItem(hDlg, IDC_RESET);
		g_hBtnReset = hBtnReset;



		SendMessage(heditFilterMsg, EM_SETLIMITTEXT, SIZE_DAT / 2, 0);

		CList = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT,
			30, 58, 330, 220, hDlg, NULL, g_hInst, NULL);
		//컬럼추가 mask 사용할목록을 마스크함
		col1.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col1.fmt = LVCFMT_LEFT;//정렬
		for (i = 0; i < 3; i++) {
			col1.cx = CLcolwidth[i];
			col1.iSubItem = i;
			col1.pszText = (LPSTR)CLcolname[i];
			ListView_InsertColumn(CList, i, &col1); //컬럼을 핸들에 추가한다
		}

		BList = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT,
			370, 58, 398, 220, hDlg, NULL, g_hInst, NULL);
		col2.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col2.fmt = LVCFMT_LEFT;//정렬
		for (i = 0; i < 2; i++) {
			col2.cx = BLcolwidth[i];
			col2.iSubItem = i;
			col2.pszText = (LPSTR)BLcolname[i];
			ListView_InsertColumn(BList, i, &col2); //컬럼을 핸들에 추가한다
		}
		EnableWindow(CList, FALSE);
		EnableWindow(BList, FALSE);
		return TRUE;

	case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);
		SetTextAlign(hdc, TA_LEFT); //왼쪽정렬
		SetBkMode(hdc, TRANSPARENT); //텍스트 배경색 윈도우색과 같게
		TextOut(hdc, 30, 30, clientlist, lstrlen(clientlist));
		TextOut(hdc, 370, 30, blacklist, lstrlen(blacklist));
		TextOut(hdc, 30, 285, filterlist, lstrlen(filterlist));
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_RESET: {
			ListView_DeleteAllItems(CList);
			InsertListViewItems(CList);
			return TRUE;
		}

		case blackAppend:
			//MessageBox(NULL, _T("IP를 입력후 이 버튼을 누르면 해당 사용자가 블랙처리됩니다."), _T("알림"), MB_ICONERROR);
			GetDlgItemTextA(hDlg, BLACKADDR, g_blackip.ip, SIZE_DAT);
			//ListView_Insert(BList, i, &col1); //컬럼을 핸들에 추가한다
			clientLI.mask = LVIF_TEXT;
			clientLI.iItem = 1;         //행 (전체 화면 - 캡션포함)
			clientLI.iSubItem = 0;    //열
			clientLI.pszText = g_blackip.ip;  //문자열 값
			ListView_InsertItem(BList, &clientLI);
			ListView_SetItemText(BList, 2, 1, g_blackip.ip);

			return TRUE;
		case blackRemv:
			MessageBox(NULL, _T("IP를 입력후 이 버튼을 누르면 해당 사용자가 블랙에서 제외됩니다."), _T("알림"), MB_ICONERROR);
			return TRUE;

		case BlackAdd:
			return TRUE;

		case filterWord:
			return TRUE;
		case filterAppend:
			if (GetDlgItemTextA(hDlg, filterWord, g_chatmsg.msg, SIZE_DAT) != 0) { //입력한게 공백이면 컨트롤에 넣지않는다.
				SendMessage(heditFilterMsg, EM_SETSEL, 0, -1);
				SetFocus(heditFilterMsg);
			}

			return TRUE;

		case filterRemove:
			MessageBox(NULL, _T("찾을 단어를 입력후 이 버튼을 누르면 해당 단어는 제외됩니다."), _T("알림"), MB_ICONERROR);
			return TRUE;

		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		//return FALSE;//??
	}
	return FALSE;


}







