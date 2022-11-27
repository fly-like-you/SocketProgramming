#pragma once

// ���� ���� �߰�
bool AddSocketInfo(SOCKET sock, bool isUDP, char* nickname)
{
	if (nTotalSockets >= FD_SETSIZE) {
		printf("[����] ���� ������ �߰��� �� �����ϴ�!\n");
		return false;
	}
	SOCKETINFO* ptr = new SOCKETINFO;
	if (ptr == NULL) {
		printf("[����] �޸𸮰� �����մϴ�!\n");
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
// ���� ���� ����
void RemoveSocketInfo(int nIndex)
{
	SOCKETINFO* ptr = SocketInfoArray[nIndex];

	if (ptr->isIPv6 == false) {
		// Ŭ���̾�Ʈ ���� ���
		struct sockaddr_in clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(ptr->sock, (struct sockaddr*)&clientaddr, &addrlen);
		// Ŭ���̾�Ʈ ���� ���
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("[TCP/IPv4 ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}
	// ���� �ݱ�
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

bool SetSock(SOCKET& listen_sock, int sockType) { // TCP, ���ŷ ����


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
