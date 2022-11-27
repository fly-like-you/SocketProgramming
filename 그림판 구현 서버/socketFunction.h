#pragma once

// 소켓 정보 추가
bool AddSocketInfo(SOCKET sock, bool isUDP, char* nickname)
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
	ptr->isUDP = isUDP;
	ptr->recvbytes = 0;
	ptr->sendbytes = 0;
	ptr->recvflag = FALSE;
	ptr->sendflag = FALSE;
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

int sendn(SOCKET s, char* buf, int len, int flags) {
	int received;
	char* ptr = buf;
	int left = len;

	while (left > 0) {
		received = send(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}

int recvn(SOCKET s, char* buf, int len, int flags)
{
	int received;
	char* ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}

bool SetSock(SOCKET& listen_sock, int sockType) { // TCP, 블로킹 소켓


	listen_sock = socket(AF_INET, sockType, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr,
		sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	if (sockType == SOCK_STREAM) {
		retval = listen(listen_sock, SOMAXCONN);
		if (retval == SOCKET_ERROR) err_quit("listen()");
	}

	return true;
}
