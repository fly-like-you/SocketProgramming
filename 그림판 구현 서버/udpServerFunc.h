#pragma once
// struct udpSocketInfo
// bool addUdpSocketInfo
int nTotalUdpSockets;

struct udpSocketInfo
{
	char   buf[BUFSIZE];
	sockaddr_in sockaddr;
};
udpSocketInfo* udpSocketInfoArray[FD_SETSIZE];




// ���� ���� �߰�
BOOL AddUdpSocketInfo(struct sockaddr_in sockaddr)
{
	if (nTotalUdpSockets >= FD_SETSIZE) {
		printf("[����] ���� ������ �߰��� �� �����ϴ�!\n");
		return FALSE;
	}

	udpSocketInfo* ptr = new udpSocketInfo;
	if (ptr == NULL) {
		printf("[����] �޸𸮰� �����մϴ�!\n");
		return FALSE;
	}

	ptr->sockaddr = sockaddr;
	udpSocketInfoArray[nTotalUdpSockets++] = ptr;

	return TRUE;
}


void getIpPort(char* ipAddress, unsigned short* sockPort, struct sockaddr_in *sockaddr) { // ���� �ּ� ����ü���� ip�ּҸ� �������� �Լ�
	char addr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &sockaddr->sin_addr, addr, sizeof(addr));
	*sockPort = sockaddr->sin_port;
	strcpy(ipAddress, addr);
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
void RemoveUdpSocketInfo(struct sockaddr_in *sockaddr)
{
	// �ε��� ã��

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