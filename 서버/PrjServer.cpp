#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기

#include "Common.h"
#include "resource.h"
#include "server.h" // 메시지 구조체 및 상수 정의 
#include "serverFunction.h" // 스레드 관련 전역변수 및 함수 정의
#include "udpServerFunc.h"   /* UDP 관련 전역 변수 및 함수 선언 헤더 */
#include "socketFunction.h"
// 콜백 프로시저
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);


// UDP, TCP 스레드 분리 이벤트
HANDLE hRecvEvent;				// 수신 이벤트와 송신 이벤트로 분리 시키기
HANDLE hSendEvent;


char* translationBuffer;        // 스레드 송수신용 전역 버퍼
HEAD_MSG    g_head_msg;         // 추가 고정 크기 메시지  헤더
BOOLEAN g_udpFlag;			    // 송신 스레드에서 TCP에서 보내는지 UDP에서 보내는지 확인하는 플래그


DWORD WINAPI DialogThread(LPVOID arg) {
	HINSTANCE hInstance = (HINSTANCE)arg;
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
	return 0;
}


// 송수신 스레드

// UDP 수신 스레드
DWORD WINAPI ClientRecvUdpThread(LPVOID arg) {								 // UDP 스레드
	int retval;
	int addrlen;
	char* buf;
	struct sockaddr_in udpAddr;

	
	// 클라이언트와 데이터 통신
	while (1) {

		retval = WaitForSingleObject(hSendEvent, INFINITE);					 // 송신 스레드의 완료를 기다리는 이벤트
		addrlen = sizeof(udpAddr);
		int index = 0;
		//필터링 
		CHAT_MSG* tmp_msg;			
																			 //고정길이 수신
		retval = recvfrom(g_udpsock, (char*)&g_head_msg, sizeof(HEAD_MSG), 0, (struct sockaddr*)&udpAddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			RemoveUdpSocketInfo(&udpAddr);
			continue;
		}

		buf = (char*)malloc(g_head_msg.length * sizeof(char));				 // 스레드 내부 임시 버퍼 초기화
		translationBuffer = (char*)malloc(g_head_msg.length * sizeof(char)); // 스레드 전달 버퍼 초기화 



		if (g_head_msg.type == TYPE_UDP_CONNECTION_REQUEST) {				// 메시지 타입: UDP가 처음 접속 했을 때
			if (BlockingUser(udpAddr)) {									// 블랙리스트 조회하여 접근 차단
				continue;
			}

													
			if (!compareUdpSocketArray(&udpAddr)) {							// 구조체를 뒤져서 없으면
				AddUdpSocketInfo(udpAddr, g_head_msg.nickname);	// 소켓 정보 구조체에 추가
			}

		}
		else if (g_head_msg.type == TYPE_UDP_DISCONNECTION) {				// 메시지 타입: UDP가 접속 종료할 때
			RemoveUdpSocketInfo(&udpAddr);									// UDP 소켓 정보 구조체에서 해당 주소를 제거함
		}
		else {																// 메시지 타입: 일반 전송
			//가변길이 수신

			retval = recvfrom(g_udpsock, (char*)buf, g_head_msg.length, 0, (struct sockaddr*)&udpAddr, &addrlen);
			if (retval == SOCKET_ERROR) {
				RemoveUdpSocketInfo(&udpAddr);
				continue;
			}
			
			tmp_msg = (CHAT_MSG*)buf;			// 필터링
			filter((char*)tmp_msg->msg, filtercount);  
		}

		for (int i = 0; i < nTotalUdpSockets; i++) {
			index = findIndexUdpSocketArray(&udpAddr);
		}
		if (index == -1) {
			continue;
		}
	
		g_head_msg.color = udpSocketInfoArray[index]->color;
		memcpy(translationBuffer, buf, g_head_msg.length);
		free(buf);

		g_udpFlag = TRUE;
		SetEvent(hRecvEvent);												// 수신 완료 상태 알림
		ResetEvent(hSendEvent);												// 이벤트가 수동 모드이기 때문에 자신의 스레드를 꺼준다
	}
	return 0;
}

// TCP 수신 스레드
DWORD WINAPI ClientRecvTcpThread(LPVOID arg) {

	// 소켓 셋 정의
	fd_set rset;
	fd_set wset;

	// 데이터 통신에 사용할 변수

	while (1) {
		
		retval = WaitForSingleObject(hSendEvent, INFINITE);						// 송신 스레드의 완료를 기다리는 이벤트
		if (retval != WAIT_OBJECT_0) break;

																				// 변수 선언
		SOCKET client_sock;
		WELCOME_MSG welcome_msg;
		struct sockaddr_in clientaddr4;
		struct sockaddr_in udpAddr;
		int addrlen;
		addrlen = sizeof(clientaddr4);
		CHAT_MSG* tmp_msg;   //필터링

																				// 소켓 셋 초기화
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_SET(g_listensock, &rset);
		
																				// 읽기 셋, 쓰기 셋 분류
		for (int i = 0; i < nTotalSockets; i++) {
			if (SocketInfoArray[i]->recvbytes > SocketInfoArray[i]->sendbytes)  // 읽기 바이트 > 쓰기 바이트이면 
				FD_SET(SocketInfoArray[i]->sock, &wset);						// 쓰기 셋 추가
			else
				FD_SET(SocketInfoArray[i]->sock, &rset);						// 아닌 경우 읽기 셋 추가
		}
		
		retval = select(0, &rset, &wset, NULL, NULL);							// select()에서 소켓의 입출력을 기다림


		if (retval == SOCKET_ERROR) { 
			err_quit("select()"); 
		}
																				// TCP 클라이언트 첫 접속

		/*------ TCP 클라이언트 연결 대기 상태 ( 소켓 정보 추가 )------*/
		// 소켓 셋에 해당 소켓에 남아 있는 경우
		if (FD_ISSET(g_listensock, &rset)) {  							
			addrlen = sizeof(clientaddr4);
			client_sock = accept(g_listensock,
				(struct sockaddr*)&clientaddr4, &addrlen);
			if (client_sock == INVALID_SOCKET) {
				err_display("accept()");
				break;
			}
			// 에러가 발생하지 않은 경우 소켓 정보 구조체에 소켓 정보를 추가
			else {

				if (BlockingUser(clientaddr4)) {								// 블랙리스트를 확인하여 유저를 차단
					closesocket(client_sock);
					continue;
				}
				retval = recvn(client_sock, (char*)&welcome_msg, sizeof(WELCOME_MSG), 0); //  TCP 첫 수신: 웰컴 메시지
				if (!AddSocketInfo(client_sock, false, welcome_msg.nickname))
					closesocket(client_sock);

			}
		}

		/*------ TCP 클라이언트로부터 메시지를 수신 ( 데이터 통신 ) ------*/
		for (int i = 0; i < nTotalSockets; i++) {
			SOCKETINFO* ptr = SocketInfoArray[i];
			// 수신 플래그가 꺼져있으면 -> 고정 크기 데이터 받기 
			if (FD_ISSET(ptr->sock, &rset) && ptr->recvflag == FALSE) {							

				//고정 크기 데이터 받기
				retval = recvn(ptr->sock, (char*)&g_head_msg, sizeof(HEAD_MSG), 0);		
				if (retval == 0) {
					RemoveSocketInfo(i);
					continue;
				}
				else if (retval == SOCKET_ERROR) {
					err_display("recv()");
					RemoveSocketInfo(i);
					continue;
				}
				
				ptr->buf = (char*)malloc(g_head_msg.length * sizeof(char));
				translationBuffer = (char*)malloc(g_head_msg.length * sizeof(char));
				ptr->recvflag = TRUE;															 // 데이터를 받고 수신 플래그를 켜줌


			}
			// 수신 플래그가 켜져있으면 -> 가변 크기 데이터 받기 
			else if (FD_ISSET(ptr->sock, &rset) && ptr->recvflag == TRUE) {						 
				retval = recvn(ptr->sock, (char*)ptr->buf, g_head_msg.length, 0);
				tmp_msg = (CHAT_MSG*)ptr->buf;													 // 필터링
				filter((char*)tmp_msg->msg, filtercount);   


				if (retval == 0) {
					RemoveSocketInfo(i);
					continue;
				}
				else if (retval == SOCKET_ERROR) {
					err_quit("recv()");
					RemoveSocketInfo(i);
					continue;
				}
				memcpy(translationBuffer, ptr->buf, retval);
				ptr->recvbytes += retval;
				ptr->recvflag = FALSE;
				free(ptr->buf);

			}
			// 수신 바이트 > 송신 바이트인경우 송신 스레드를 통해 데이터 전송
			if (FD_ISSET(ptr->sock, &wset)) {  
				
				g_head_msg.color = ptr->color;
				SetEvent(hRecvEvent);											// 송신 스레드 활성화
				ResetEvent(hSendEvent);											// 수동 리셋 모드이기 때문에 이벤트 종료
			}
		} /* 반복문 종료 */

	} /* 무한 반복 종료 */
	return 0;
}

// 클라이언트 송신 스레드 모든 사용자에게 데이터 전송
// TCP는 송신스레드를 두번에 걸쳐서 데이터를 전송
// UDP는 한번에 데이터를 전송
DWORD WINAPI ClientSendThread(LPVOID arg) {
	
	// TCP, UDP간 송신 횟수를 조절하기 위한 플래그
	BOOLEAN udpFlag = FALSE;

	while (1) {
		// 수신 스레드 이벤트 완료 시
		retval = WaitForSingleObject(hRecvEvent, INFINITE);										
		if (retval != WAIT_OBJECT_0) break;

		// 1. TCP 전송
		for (int j = 0; j < nTotalSockets; j++) {
			if (g_head_msg.type == TYPE_UDP_CONNECTION_REQUEST || g_head_msg.type == TYPE_UDP_DISCONNECTION) break;		// UDP 타입으로 들어온 데이터를 거르기
			SOCKETINFO* ptr2 = SocketInfoArray[j];
			if (ptr2->sendflag == FALSE) {																				// 수신 플래그가 꺼진경우 -> 고정 길이 전송
				retval = sendn(ptr2->sock, (char*)&g_head_msg, sizeof(HEAD_MSG), 0);							
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					RemoveSocketInfo(j);
					--j; // 루프 인덱스 보정
					continue;
				}

				ptr2->sendflag = TRUE;
			}
																														// 수신 플래그가 켜진경우 -> 가변 길이 전송
			else if (ptr2->sendflag == TRUE) {																			// UDP 수신 스레드에서 전송한 경우 해당 if문은 작동하지 않음 ( UDP는 한번에 모두 전송하는 반면 TCP는 두번에 결쳐서 전송하기 때문)
				retval = sendn(ptr2->sock, (char*)translationBuffer, g_head_msg.length, 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					RemoveSocketInfo(j);
					--j; // 루프 인덱스 보정
					continue;
				}
																														// 송수신 바이트 멤버 컨트롤
				ptr2->sendbytes += retval;																					// 보낸 데이터만큼 수신 바이트 추가
				if (ptr2->recvbytes <= ptr2->sendbytes) {																	// 송수신 바이트가 같은 경우 ( 메시지를 모두 전송한 경우 )
					ptr2->recvbytes = ptr2->sendbytes = 0;																		// 송수신 바이트 0으로 초기화
				}									
				ptr2->sendflag = FALSE;																					// 플래그 초기화
				udpFlag = TRUE;
			}

		}
		
		// 플래그 초기화
		if (nTotalSockets == 0 || g_udpFlag == TRUE)
			udpFlag = TRUE;

		
		// 2. UDP 전송
		if (udpFlag == TRUE) {
			
			for (int k = 0; k < nTotalUdpSockets; k++) {																// 모든 UDP 소켓에 대하여 전송
				udpSocketInfo* ptrUdp = udpSocketInfoArray[k];
																													//고정길이 전송
				retval = sendto(g_udpsock, (char*)&g_head_msg, sizeof(HEAD_MSG), 0, (struct sockaddr*)&ptrUdp->sockaddr, sizeof(ptrUdp->sockaddr));
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					RemoveUdpSocketInfo(&ptrUdp->sockaddr);
					--k; 
					continue;
				}

																																
				if (g_head_msg.type == TYPE_UDP_CONNECTION_REQUEST) {													// UDP 메시지 타입이 접속요청, 연결해제가 아닌 경우에만 전송
				}
				else if (g_head_msg.type == TYPE_UDP_DISCONNECTION) {

				}
				else {																									// 가변길이 전송
					retval = sendto(g_udpsock, (char*)translationBuffer, g_head_msg.length, 0, (struct sockaddr*)&ptrUdp->sockaddr, sizeof(ptrUdp->sockaddr));
					if (retval == SOCKET_ERROR) {
						err_display("send()");
						RemoveUdpSocketInfo(&ptrUdp->sockaddr);
						--k; 
						continue;
					}
					
					if (g_udpFlag == TRUE) {																			// 전역 플래그가 TRUE인경우 (21번 주석)-> UDP에서 전송한 경우
						for (int j = 0; j < nTotalSockets; j++) {														// TCP에서 미처 전송하지 못한 가변 길이 데이터를 보냄
							SOCKETINFO* ptr2 = SocketInfoArray[j];
							retval = sendn(ptr2->sock, (char*)translationBuffer, g_head_msg.length, 0);
							if (retval == SOCKET_ERROR) {
								err_display("send()");
								RemoveSocketInfo(j);
								--j; // 루프 인덱스 보정
								continue;
							}

							ptr2->sendbytes += retval;																	
							if (ptr2->recvbytes <= ptr2->sendbytes) {
								ptr2->recvbytes = ptr2->sendbytes = 0;
							}
							ptr2->sendflag = FALSE;																		
						}
					}
					
					g_udpFlag = FALSE;																					// 플래그 초기화
				}

			}

			udpFlag = FALSE;

		}


		// 수신 완료 신호 전송
		SetEvent(hSendEvent);
		// 수동 모드이기 때문에 자신의 이벤트를 종료
		ResetEvent(hRecvEvent);
	}
	return 0;
}


// 통신 서버 초기 설정용 메인 함수
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;


	/* TCP, UDP 소켓 생성 */
	SetSock(g_listensock, SOCK_STREAM);
	SetSock(g_udpsock, SOCK_DGRAM);

	/* 논 블로킹 소켓 적용 */
	u_long on = 1;
	retval = ioctlsocket(g_listensock, FIONBIO, &on);
	if (retval == SOCKET_ERROR) {
		err_quit("ioctlsocket()");
	}
	/* 이벤트 객체 처리 (수동 모드) */
	hRecvEvent = CreateEvent(NULL, TRUE, FALSE, NULL);  // 클라이언트로 부터 데이터 받기 완료
	hSendEvent = CreateEvent(NULL, TRUE, FALSE, NULL);  // 클라이언트로 부터 데이터 전송 완료

	HANDLE hClientRecvUdpThread = CreateThread(NULL, 0, ClientRecvUdpThread, NULL, 0, NULL);
	HANDLE hClientRecvTcpThread = CreateThread(NULL, 0, ClientRecvTcpThread, NULL, 0, NULL);
	HANDLE hClientWriteThread = CreateThread(NULL, 0, ClientSendThread, NULL, 0, NULL);

	// 수신 스레드 완료
	SetEvent(hSendEvent);

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

	closesocket(g_listensock);
	closesocket(g_udpsock);

	WSACleanup();
	return 0;
}

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {


	/* 각종 핸들*/
	static HWND heditIPaddr;
	static HWND hBtnblackAppend; // 전역 변수에도 저장
	static HWND hBtnblackRemove; // 전역 변수에도 저장
	static HWND hBtnfilterAppend; // 전역 변수에도 저장
	static HWND hBtnfilterRemove; // 전역 변수에도 저장
	static HWND hBtnReset;
	static HWND heditFilterMsg;

	static HWND hBtnEvery;
	static HWND heditEveryMsg;


	/* paint를 위한 변수들 */
	HDC hdc;
	PAINTSTRUCT ps;
	const char* clientlist = "클라이언트 목록";
	const char* blacklist = "블랙리스트 목록";
	const char* filterlist = "금지 단어";
	const char* noticeTitle = "공지사항";

	/* 클라이언트, 블랙리스트 목록들 위한 변수들*/
	static HWND CList; //클라이언트 목록 리스트
	static HWND BList; //블랙리스트 목록
	static HWND FList; //금지단어 목록
	LVCOLUMN col1; //Clinetlist 컬럼
	LVCOLUMN col2; //Blacklist 컬럼
	LVCOLUMN col3;  //Filterlist 컬럼
	LVITEM clientLI;


	int CLcolwidth[3] = { 90, 150, 90 };
	LPWSTR CLcolname[3] = { (LPWSTR)TEXT("id"), (LPWSTR)TEXT("IP") , (LPWSTR)TEXT("색") };

	int BLcolwidth[1] = { 398 };
	LPWSTR BLcolname[1] = { (LPWSTR)TEXT("IP") };


	int FLcolwidth[1] = { 330 };
	LPWSTR FLcolname[1] = { (LPWSTR)TEXT("단어") };


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


		hBtnReset = GetDlgItem(hDlg, IDC_RESET);
		g_hBtnReset = hBtnReset;
		hBtnEvery = GetDlgItem(hDlg, IDC_EVERYBTN);
		heditEveryMsg = GetDlgItem(hDlg, IDC_EVERYUSER);



		SendMessage(heditFilterMsg, EM_SETLIMITTEXT, SIZE_DAT / 2, 0);

		CList = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT,
			30, 58, 330, 190, hDlg, NULL, g_hInst, NULL);
		//컬럼추가 mask 사용할목록을 마스크함
		col1.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col1.fmt = LVCFMT_LEFT;//정렬
		for (int i = 0; i < 3; i++) {
			col1.cx = CLcolwidth[i];
			col1.iSubItem = i;
			col1.pszText = (LPSTR)CLcolname[i];
			ListView_InsertColumn(CList, i, &col1); //컬럼을 핸들에 추가한다
		}

		BList = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT,
			370, 58, 380, 190, hDlg, NULL, g_hInst, NULL);
		col2.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col2.fmt = LVCFMT_LEFT;//정렬
		for (int i = 0; i < 1; i++) {
			col2.cx = BLcolwidth[i];
			col2.iSubItem = i;
			col2.pszText = (LPSTR)BLcolname[i];
			ListView_InsertColumn(BList, i, &col2); //컬럼을 핸들에 추가한다
		}

		FList = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT,
			30, 285, 330, 100, hDlg, NULL, g_hInst, NULL);
		col3.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col3.fmt = LVCFMT_LEFT;//정렬
		for (int i = 0; i < 1; i++) {
			col3.cx = FLcolwidth[i];
			col3.iSubItem = i;
			col3.pszText = (LPSTR)FLcolname[i];
			ListView_InsertColumn(FList, i, &col3); //컬럼을 핸들에 추가한다
		}
		EnableWindow(CList, TRUE);
		EnableWindow(BList, TRUE);
		EnableWindow(FList, TRUE);

		return TRUE;


	case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);
		SetTextAlign(hdc, TA_LEFT); //왼쪽정렬
		SetBkMode(hdc, TRANSPARENT); //텍스트 배경색 윈도우색과 같게
		TextOut(hdc, 30, 30, clientlist, lstrlen(clientlist));
		TextOut(hdc, 370, 30, blacklist, lstrlen(blacklist));
		TextOut(hdc, 30, 257, filterlist, lstrlen(filterlist));
		TextOut(hdc, 370, 257, noticeTitle, lstrlen(noticeTitle));
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_RESET: {
			ListView_DeleteAllItems(CList);
			InsertListViewItems(CList);
			return TRUE;
		}

		case blackAppend:
			ListView_DeleteAllItems(BList);
			GetDlgItemTextA(hDlg, BLACKADDR, blackaddr, INET_ADDRSTRLEN);
			AddBlackUser(blackaddr);
			ShowBlackuser(BList);
			memset(blackaddr, 0, sizeof(char));

			return TRUE;
		case blackRemv:
			ListView_DeleteAllItems(BList);
			GetDlgItemTextA(hDlg, BLACKADDR, blackaddr, INET_ADDRSTRLEN);
			RemoveBlackUser(blackaddr);
			ShowBlackuser(BList);
			memset(blackaddr, 0, sizeof(char));
			return TRUE;

		case BlackAdd:
			return TRUE;

		case filterWord:
			return TRUE;
		case filterAppend:
			ListView_DeleteAllItems(FList);
			GetDlgItemTextA(hDlg, filterWord, tmpfilter, MAX_FILTERWORD_SIZE);
			addfilter(tmpfilter);
			Showfilter(FList);
			memset(tmpfilter, 0, sizeof(char));



			return TRUE;

		case filterRemove:
			ListView_DeleteAllItems(FList);
			GetDlgItemTextA(hDlg, filterWord, tmpfilter, MAX_FILTERWORD_SIZE);
			removefilter(tmpfilter);
			Showfilter(FList);
			memset(tmpfilter, 0, sizeof(char));
			return TRUE;


		case IDC_EVERYBTN:                   // 공지사항 전달하는 버튼
			if (nTotalSockets > 0 || nTotalUdpSockets > 0) {
				memset(g_chatmsg.msg, 0, sizeof(char) * SIZE_DAT);
				GetDlgItemTextA(hDlg, IDC_EVERYUSER, g_chatmsg.msg, SIZE_DAT);         // 채팅 메시지
				sendAll(g_chatmsg);

			}
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		
	}
	return FALSE;


}


