#pragma once

BOOL BlockingUser(struct sockaddr_in sockaddr) {
	unsigned short port;
	char ipAddress[INET_ADDRSTRLEN];
	char int2stringPort[6] = { 0. };

	// ����� ip �ּ� �޾ƿ���
	getIpPort(ipAddress, &port, &sockaddr);
	for (int i = 0; i < TotalBlackUser; i++) {  // ������Ʈ �迭�� ���ؼ�
		if (!strcmp(ipAddress, g_blackinfo[i].addr)) {  // �迭�� �����ϸ�
			MessageBoxA(NULL, (const char*)"�㰡���� ���� ����� 1���� ���ܵǾ����ϴ�.", (const char*)"Administrator", MB_ICONERROR);
			return TRUE;  // TRUE ����
		}
	}
	// �迭�� �������� ������ 
	// FAlSE ����
	return FALSE;
}


// ���� ���� �߰�
int AddUdpSocketInfo(struct sockaddr_in sockaddr, char* nickname)
{
	if (nTotalUdpSockets >= FD_SETSIZE) {
		printf("[����] ���� ������ �߰��� �� �����ϴ�!\n");
		return -1;
	}

	udpSocketInfo* ptr = new udpSocketInfo;
	if (ptr == NULL) {
		printf("[����] �޸𸮰� �����մϴ�!\n");
		return -1;
	}

	int color;
	strcpy(ptr->nickname, nickname);
	ptr->sockaddr = sockaddr;
	ptr->color = getRandomColor();
	color = ptr->color;
	udpSocketInfoArray[nTotalUdpSockets++] = ptr;

	return color;
}





BOOL compareUdpSocketArray(struct sockaddr_in* target) {
	for (int i = 0; i < nTotalUdpSockets; i++) {
		udpSocketInfo* ptr = udpSocketInfoArray[i];

		// ���� ����ü
		char ipAddress[INET_ADDRSTRLEN];
		unsigned short port;
		getIpPort(ipAddress, &port, &ptr->sockaddr);

		// �ε��� ã�� ��� ����ü
		char targetAddr[INET_ADDRSTRLEN];
		unsigned short targetPort;
		getIpPort(targetAddr, &targetPort, target);

		// IP�ּҿ� ��Ʈ��ȣ�� ��ġ�ϸ� 
		if (strcmp(ipAddress, targetAddr) == 0 && port == targetPort) {
			return TRUE;
		}
	}
	return FALSE;
}
int findIndexUdpSocketArray(struct sockaddr_in* target) {
	for (int i = 0; i < nTotalUdpSockets; i++) {
		udpSocketInfo* ptr = udpSocketInfoArray[i];

		// ���� ����ü
		char ipAddress[INET_ADDRSTRLEN];
		unsigned short port;
		getIpPort(ipAddress, &port, &ptr->sockaddr);

		// �ε��� ã�� ��� ����ü
		char targetAddr[INET_ADDRSTRLEN];
		unsigned short targetPort;
		getIpPort(targetAddr, &targetPort, target);

		// IP�ּҿ� ��Ʈ��ȣ�� ��ġ�ϸ� �ε��� ��ȯ
		if (strcmp(ipAddress, targetAddr) == 0 && port == targetPort) {
			return i;
		}
	}
	return -1;

}
// ���� ���� ����
void RemoveUdpSocketInfo(struct sockaddr_in* sockaddr)
{
	// �ε��� ã��
	if (BlockingUser(*sockaddr)) {
		return;
	}
	int nIndex = findIndexUdpSocketArray(sockaddr);
	if (nIndex == -1) {
		MessageBox(NULL, _T("�ε����� ã�� �� �����ϴ�"), _T("�˸�"), MB_ICONERROR);
	}
	udpSocketInfo* ptr = udpSocketInfoArray[nIndex];

	// �ش� �ε��� ���� �����ϱ�
	delete ptr;


	// �ش� �ε����� �迭 ������ ���� �־��ֱ�
	if (nIndex != (nTotalUdpSockets - 1))
		udpSocketInfoArray[nIndex] = udpSocketInfoArray[nTotalUdpSockets - 1];

	--nTotalUdpSockets;
}