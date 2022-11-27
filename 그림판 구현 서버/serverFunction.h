#pragma once

int retval;

int size = 1;
char nickname[MAX_USER][MAX_NAME_SIZE] = {"xxx","yyy","zzz"};




/* 전역 변수 선언 */
SOCKET			   g_listensock;
SOCKET			   g_udpsock;
static CHAT_MSG    g_chatmsg;         // 채팅 메시지
static BLACK_IP    g_blackip;         //차단 ip




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

