#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기

#include "Common.h"
#include "resource.h"
#include "server.h" // 메시지 구조체 및 상수 정의 
#include "serverFunction.h" // 스레드 관련 전역변수 및 함수 정의

// 콜백 프로시저
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

DWORD WINAPI DialogThread(LPVOID arg) {
	HINSTANCE hInstance = (HINSTANCE)arg;
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
	return 0;
}
DWORD WINAPI ConnectionThread(LPVOID arg) {
	// 첫 접속 시에는 이름과 펜 색깔을 정해줘야 함
	//블랙리스트 처리
	// 공지사항 전파
	fd_set rset;
	SOCKET client_sock;
	int addrlen;
	// 데이터 통신에 사용할 변수(IPv4)


	while (1) {
		// 박준호 추가
		HEAD_MSG    head_msg;         // 박준호 추가 고정 크기 메시지  헤더
		struct sockaddr_in clientaddr4;
		WELCOME_MSG welcome_msg;
		// 소켓 셋 초기화

		FD_ZERO(&rset);
		FD_SET(g_listensock, &rset);
		FD_SET(g_udpsock, &rset); // UDP 넣기
		for (int i = 0; i < nTotalSockets; i++) {
			FD_SET(SocketInfoArray[i]->sock, &rset);
		}

		// select()
		retval = select(0, &rset, NULL, NULL, NULL);

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
		if (FD_ISSET(g_udpsock, &rset)) {
			if (!AddSocketInfo(g_udpsock, true, (char*)""))
				closesocket(g_udpsock);
		}

		// 소켓 셋 검사(2): 데이터 통신
		for (int i = 0; i < nTotalSockets; i++) {
			SOCKETINFO* ptr = SocketInfoArray[i];
			if (FD_ISSET(ptr->sock, &rset)) {
				if (!ptr->isUDP) {
					//  박준호 추가 고정 크기 데이터 받기
					retval = recv(ptr->sock, (char*)&head_msg, sizeof(HEAD_MSG), MSG_WAITALL);
					if (retval == 0 || retval == SOCKET_ERROR) {
						RemoveSocketInfo(i);
						continue;
					}
					ptr->buf = (char*)malloc(head_msg.length * sizeof(char));
					retval = recv(ptr->sock, (char*)ptr->buf, head_msg.length, MSG_WAITALL);

					if (retval == 0 || retval == SOCKET_ERROR) {
						RemoveSocketInfo(i);
						continue;
					}
				}
				else { // UDP인 경우
					addrlen = sizeof(clientaddr4);
					retval = recvfrom(ptr->sock, ptr->buf, sizeof(ptr->buf), 0, (struct sockaddr*)&clientaddr4, &addrlen);
					char addr[INET_ADDRSTRLEN];
					inet_ntop(AF_INET, &clientaddr4.sin_addr, addr, sizeof(addr));
					printf("\n[UDP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
						addr, ntohs(clientaddr4.sin_port));
					if (retval == 0 || retval == SOCKET_ERROR) {
						RemoveSocketInfo(i);
						continue;
					}
					ptr->recvbytes = retval;
				}

				// 현재 접속한 모든 클라이언트에 데이터 전송
				for (int j = 0; j < nTotalSockets; j++) {
					SOCKETINFO* ptr2 = SocketInfoArray[j];

					retval = send(ptr2->sock, (char*)&head_msg, sizeof(HEAD_MSG), 0); // 받은 데이터를 그대로 돌려주기 때문에 head_msg 재사용 서버에서 처리를 하는 경우에는 바껴야댐
					if (retval == SOCKET_ERROR) {
						err_display("send()");
						RemoveSocketInfo(j);
						--j; // 루프 인덱스 보정
						continue;
					}
					// 헤더에서 받은 길이는 보내는 만큼의 크기
					retval = send(ptr2->sock, (char*)ptr->buf, head_msg.length, 0);
					if (retval == SOCKET_ERROR) {
						err_display("send()");
						RemoveSocketInfo(j);
						--j; // 루프 인덱스 보정
						continue;
					}
				}
				free(ptr->buf);
			}
		} /* end of for */

	} /* end of while (1) */
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

	HANDLE hThread2 = CreateThread(NULL, 0, ConnectionThread, NULL, 0, NULL);
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







