#pragma once


struct udpSocketInfo
{
	char   nickname[MAX_NAME_SIZE];
	char   buf[BUFSIZE];
	int	   color;
	sockaddr_in sockaddr;
};
udpSocketInfo* udpSocketInfoArray[FD_SETSIZE];



int nTotalUdpSockets = 0;
int retval;

/* 전역 변수 선언 */
SOCKET			   g_listensock;
SOCKET			   g_udpsock;

static CHAT_MSG    g_chatmsg;         // 채팅 메시지
static int filtercount = 0; //필터된 단어 수
static int TotalBlackUser = 0; // 전체 블랙 유저 수
static int nTotalSockets = 0;
static BLACK_INFO    g_blackinfo[MAX_BLACK_USER];         //차단 ip
static char g_filterarray[MAX_FILTERARRAY_SIZE][MAX_FILTERWORD_SIZE];				//// 금지단어 배열 선언


SOCKETINFO* SocketInfoArray[FD_SETSIZE];


/* 윈도우 관련 전역 변수*/
static HINSTANCE  g_hInst; //프로그램 인스턴스 핸들
static HWND g_hBtnblackAppend; //블랙리스트 추가 버튼
static HWND g_hBtnblackRemove; //블랙리스트 제거 버튼
static HWND g_hBtnfilterAppend; //금지단어 추가버튼
static HWND g_hBtnfilterRemove; //금지단어 제거버튼
static HWND g_hBtnReset; //클라이언트 목록 새로고침;
static HWND g_hEditStatus;   // 각종 메시지 출력 영역
void RemoveSocketInfo(int nIndex);
void RemoveUdpSocketInfo(struct sockaddr_in* sockaddr);
int getRandomColor();



void getIpPort(char* ipAddress, unsigned short* sockPort, struct sockaddr_in* sockaddr) { // 소켓 주소 구조체에서 ip주소를 가져오는 함수
	char addr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &sockaddr->sin_addr, addr, sizeof(addr));
	*sockPort = sockaddr->sin_port;
	strcpy(ipAddress, addr);
}
void filter(char censoredSentence[], int count) {            ////  필터링
	int i = 0;
	if (count > 0) {
		for (int j = 0; j < count; j++) {

			char* ptr = strstr(censoredSentence, g_filterarray[j]); //문장과 욕설단어 비교해서 일치시 포인터 반환

			while (ptr != NULL) {    //일치하는 단어가 없을때 까지
				ptr = strstr(censoredSentence + i, g_filterarray[j]);
				if (!ptr) return;
				int length = strlen(g_filterarray[j]);
				char* asterisk = (char*)malloc(length * sizeof(char));   //치환할 단어의 길이만큼 *(asterisk) 생성
				memset(asterisk, '*', length); 

				strncpy(ptr, asterisk, length); 
				i++;
				free(asterisk);

			}

		}
	}
}

BOOL AddBlackUser(char* blackAddress) {



	// 서버 칸에서 작성한 아이피 주소를 받아온다 blackaddress
	// 받아온 아이피 주소를 리스트에 추가 시킨다.


	strcpy(g_blackinfo[TotalBlackUser].addr, blackAddress);
	TotalBlackUser++;

	return TRUE;
}
void RemoveBlackUser(char* blackaddress) {
	for (int i = 0; i < TotalBlackUser; i++) {
		if (strcmp(blackaddress, g_blackinfo[i].addr) == 0) {
			--TotalBlackUser;
			for (int j = i; j < TotalBlackUser; j++) {
				g_blackinfo[j] = g_blackinfo[j + 1];     //삭제 후 배열을 다시 정렬
			}

		}
	}
}


BOOL ShowBlackuser(HWND hWndListView) {    //블랙리스트 목록에 현재까지 차단당한 ip들을 보여준다.
	LVITEM lvI;
	lvI.pszText = NULL;
	lvI.mask = LVIF_TEXT;
	lvI.stateMask = 0;
	lvI.iSubItem = 0;
	lvI.state = 0;


	for (int i = 0; i < TotalBlackUser; i++) {

		lvI.iItem = i;


		if (ListView_InsertItem(hWndListView, &lvI) == -1) return FALSE;
		ListView_SetItemText(hWndListView, i, 0, g_blackinfo[i].addr);
	}

	return TRUE;
}

BOOL InsertListViewItems(HWND hWndListView)        //현재까지 접속한 tcp, udp 클라이언트들의 정보 출력
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
		char tmpNickname[MAX_NAME_SIZE];
		char nickname[MAX_NAME_SIZE + 7] =  "[TCP] ";
		char cred[MAX_NAME_SIZE] = { 0, };   
		char cgreen[MAX_NAME_SIZE] = { 0, };
		char cblue[MAX_NAME_SIZE] = { 0, };
		int red = (ptr->color >> 16) & 0xFF;    //RGB의 R값을 나타냄
		int green = (ptr->color >> 8) & 0xFF;   //RGB의 G값을 나타냄
		int blue = ptr->color & 0xFF;			//RGB의 B값을 나타냄
		sprintf(cred, "(%d", red);     //정수R값을 문자열로 바꿈
		sprintf(cgreen, ",%d", green);
		sprintf(cblue, ",%d)", blue);
		strcat(cred, cgreen);		//앞의 배열에 뒤의 배열을 이어 붙임
		strcat(cred, cblue);

		strcpy(tmpNickname, ptr->nickname);
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		//Insert items into the list.
		if (ListView_InsertItem(hWndListView, &lvI) == -1)
			return FALSE;

		strcat(nickname, tmpNickname);


		ListView_SetItemText(hWndListView, i, 0, nickname);
		ListView_SetItemText(hWndListView, i, 1, addr);
		ListView_SetItemText(hWndListView, i, 2, cred);
	}

	for (int j = 0; j < nTotalUdpSockets; j++) {
		lvI.iItem = j;
		udpSocketInfo* ptrUdp = udpSocketInfoArray[j];
		char addr[INET_ADDRSTRLEN];
		char tmpNickname[MAX_NAME_SIZE];
		char nickname[MAX_NAME_SIZE + 7] = "[UDP] ";
		char cred[MAX_NAME_SIZE] = { 0, };
		char cgreen[MAX_NAME_SIZE] = { 0, };
		char cblue[MAX_NAME_SIZE] = { 0, };
		int red = (ptrUdp->color >> 16) & 0xFF;
		int green = (ptrUdp->color >> 8) & 0xFF;
		int blue = ptrUdp->color & 0xFF;
		sprintf(cred, "(%d", red);
		sprintf(cgreen, ",%d", green);
		sprintf(cblue, ",%d)", blue);
		strcat(cred, cgreen);
		strcat(cred, cblue);
		unsigned short port;
		getIpPort(addr, &port, &ptrUdp->sockaddr);
		strcpy(tmpNickname, ptrUdp->nickname);
		if (ListView_InsertItem(hWndListView, &lvI) == -1)
			return FALSE;

		strcat(nickname, tmpNickname);

		ListView_SetItemText(hWndListView, j, 0, nickname);
		ListView_SetItemText(hWndListView, j, 1, addr);
		ListView_SetItemText(hWndListView, j, 2, cred);
	}
	return TRUE;
}

void sendAll(CHAT_MSG chatmsg) {                       // 공지사항
	HEAD_MSG    head_msg;
	// 현재 접속한 모든 클라이언트에 데이터 전송
	head_msg.type = TYPE_CHAT;
	head_msg.length = sizeof(chatmsg);
	memset(chatmsg.nickname, 0, strlen(chatmsg.nickname));
	strcpy(chatmsg.nickname, "공지사항");
	for (int i = 0; i < nTotalSockets; i++) {

		SOCKETINFO* ptr = SocketInfoArray[i];
		retval = send(ptr->sock, (char*)&head_msg, sizeof(HEAD_MSG), 0); //고정 크기 메세지 헤더
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			RemoveSocketInfo(i);
			--i; // 루프 인덱스 보정
			continue;
		}
		retval = send(ptr->sock, (char*)&chatmsg, head_msg.length, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			RemoveSocketInfo(i);
			--i; // 루프 인덱스 보정
			continue;
		}
	}

	for (int j = 0; j < nTotalUdpSockets; j++) {
		udpSocketInfo* ptrUdp = udpSocketInfoArray[j];
		retval = sendto(g_udpsock, (char*)&head_msg, sizeof(HEAD_MSG), 0, (struct sockaddr*)&ptrUdp->sockaddr, sizeof(ptrUdp->sockaddr));
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			RemoveUdpSocketInfo(&ptrUdp->sockaddr);
			--j; // 루프 인덱스 보정
			continue;
		}

		retval = sendto(g_udpsock, (char*)&chatmsg, head_msg.length, 0, (struct sockaddr*)&ptrUdp->sockaddr, sizeof(ptrUdp->sockaddr));
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			RemoveUdpSocketInfo(&ptrUdp->sockaddr);
			--j; // 루프 인덱스 보정
			continue;
		}
	}
}









void addfilter(char* addfilter) {			//금지단어 추가

	strcpy(g_filterarray[filtercount], addfilter);
	filtercount++;

}


BOOL Showfilter(HWND hWndListView) {		//현재까지 추가한 금지단어 보여줌


	LVITEM lvI;
	lvI.pszText = NULL;
	lvI.mask = LVIF_TEXT;
	lvI.stateMask = 0;
	lvI.iSubItem = 0;
	lvI.state = 0;



	for (int i = 0; i < filtercount; i++) {

		lvI.iItem = i;

		if (ListView_InsertItem(hWndListView, &lvI) == -1) return FALSE;
		ListView_SetItemText(hWndListView, i, 0, g_filterarray[i]);
	}

	return TRUE;
}


void removefilter(char* removefilter) {			//금지단어 삭제
	for (int i = 0; i < filtercount; i++) {
		if (strcmp(removefilter, g_filterarray[i]) == 0) {
			--filtercount;
			for (int j = i; j < filtercount; j++) {
				strcpy(g_filterarray[j], g_filterarray[j + 1]);
			}

		}
	}
}