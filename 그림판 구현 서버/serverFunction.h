#pragma once

int retval;

int size = 1;
char nickname[MAX_USER][MAX_NAME_SIZE] = {"xxx","yyy","zzz"};




/* 전역 변수 선언 */
SOCKET			   g_listensock;
static CHAT_MSG    g_chatmsg;         // 채팅 메시지
static BLACK_IP    g_blackip;         //차단 ip
static CLIENT_INFO g_client_info[5];  //클라이언트 정보



static int nTotalSockets = 0;

SOCKETINFO* SocketInfoArray[FD_SETSIZE];
int client_conn = 0;


/* 윈도우 관련 전역 변수*/
static HINSTANCE  g_hInst; //프로그램 인스턴스 핸들
static HWND g_hBtnblackAppend; //블랙리스트 추가 버튼
static HWND g_hBtnblackRemove; //블랙리스트 제거 버튼
static HWND g_hBtnfilterAppend; //금지단어 추가버튼
static HWND g_hBtnfilterRemove; //금지단어 제거버튼
static HWND g_hBtnReset; //클라이언트 목록 새로고침;
static HWND g_hEditStatus;   // 각종 메시지 출력 영역


// 소켓 정보 관리 함수
bool AddSocketInfo(SOCKET sock, bool isIPv6, bool isUDP);
void RemoveSocketInfo(HWND hwnd, int nIndex);
BOOL InsertListViewItems(HWND hWndListView, int cItems);



bool CreateListenSock(SOCKET& listen_sock) { // TCP, 블로킹 소켓

	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr,
		sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");
	return true;
}

// 소켓 정보 추가
bool AddSocketInfo(SOCKET sock, bool isIPv6, bool isUDP, char* nickname)
{
	if (nTotalSockets >= FD_SETSIZE) {
		printf("[오류] 소켓 정보를 추가할 수 없습니다!\n");
		return false;
	}
	SOCKETINFO* ptr = new SOCKETINFO;
	if (ptr == NULL) {
		printf("[오류] 메모리가 부족합니다!\n");
		return false;
	}
	ptr->sock = sock;
	ptr->isIPv6 = isIPv6;
	ptr->isUDP = isUDP;
	ptr->recvbytes = 0;
	strcpy(ptr->nickname, nickname);
	SocketInfoArray[nTotalSockets++] = ptr;
	return true;
}
// 소켓 정보 삭제
void RemoveSocketInfo(int nIndex)
{
	SOCKETINFO* ptr = SocketInfoArray[nIndex];

	if (ptr->isIPv6 == false) {
		// 클라이언트 정보 얻기
		struct sockaddr_in clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(ptr->sock, (struct sockaddr*)&clientaddr, &addrlen);
		// 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("[TCP/IPv4 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}
	// 소켓 닫기
	//ListView_DeleteItem(nIndex);
	closesocket(ptr->sock);
	delete ptr;

	if (nIndex != (nTotalSockets - 1))
		SocketInfoArray[nIndex] = SocketInfoArray[nTotalSockets - 1];
	--nTotalSockets;
}

// 리스트뷰에 항목 넣기
BOOL InsertListViewItems(HWND hWndListView)
{
	LVITEM lvI;

	//Initialize LVITEM members that are common to all items.
	lvI.pszText = NULL; //Sends an LVN_GETDISPINFO message.
	lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
	lvI.stateMask = 0;
	lvI.iSubItem = 0;
	lvI.state = 0;

	struct sockaddr_in clientaddr;
	int addrlen;
	addrlen = sizeof(clientaddr);
	for (int i = 0; i < nTotalSockets; i++) {
		SOCKETINFO* ptr = SocketInfoArray[i];

		lvI.iItem = i;

		getpeername(ptr->sock, (struct sockaddr*)&clientaddr, &addrlen);
		char addr[INET_ADDRSTRLEN];
		char nickname[MAX_NAME_SIZE];
		strcpy(nickname, ptr->nickname);
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		//Insert items into the list.
		if (ListView_InsertItem(hWndListView, &lvI) == -1)
			return FALSE;
		ListView_SetItemText(hWndListView, i, 0, nickname);
		ListView_SetItemText(hWndListView, i, 1, addr);
		
	}

	return TRUE;
}

