#pragma once
// struct udpSocketInfo
// bool addUdpSocketInfo







// 소켓 정보 추가
BOOL AddUdpSocketInfo(struct sockaddr_in sockaddr)
{
	if (nTotalUdpSockets >= FD_SETSIZE) {
		printf("[오류] 소켓 정보를 추가할 수 없습니다!\n");
		return FALSE;
	}

	udpSocketInfo* ptr = new udpSocketInfo;
	if (ptr == NULL) {
		printf("[오류] 메모리가 부족합니다!\n");
		return FALSE;
	}

	ptr->sockaddr = sockaddr;
	udpSocketInfoArray[nTotalUdpSockets++] = ptr;

	return TRUE;
}





BOOL compareUdpSocketArray(struct sockaddr_in* target) {
	for (int i = 0; i < nTotalUdpSockets; i++) {
		udpSocketInfo* ptr = udpSocketInfoArray[i];

		// 현재 구조체
		char ipAddress[INET_ADDRSTRLEN];
		unsigned short port;
		getIpPort(ipAddress, &port, &ptr->sockaddr);

		// 인덱스 찾는 대상 구조체
		char targetAddr[INET_ADDRSTRLEN];
		unsigned short targetPort;
		getIpPort(targetAddr, &targetPort, target);

		// IP주소와 포트번호가 일치하면 
		if (strcmp(ipAddress, targetAddr) == 0 && port == targetPort) {
			return TRUE;
		}
	}
	return FALSE;
}
int findIndexUdpSocketArray(struct sockaddr_in* target) {
	for (int i = 0; i < nTotalUdpSockets; i++) {
		udpSocketInfo* ptr = udpSocketInfoArray[i];

		// 현재 구조체
		char ipAddress[INET_ADDRSTRLEN];
		unsigned short port;
		getIpPort(ipAddress, &port, &ptr->sockaddr);

		// 인덱스 찾는 대상 구조체
		char targetAddr[INET_ADDRSTRLEN];
		unsigned short targetPort;
		getIpPort(targetAddr, &targetPort, target);

		// IP주소와 포트번호가 일치하면 인덱스 반환
		if (strcmp(ipAddress, targetAddr) == 0 && port == targetPort) {
			return i;
		}
	}
	return -1;

}
// 소켓 정보 삭제
void RemoveUdpSocketInfo(struct sockaddr_in *sockaddr)
{
	// 인덱스 찾기

	int nIndex = findIndexUdpSocketArray(sockaddr);
	if (nIndex == -1) {
		MessageBox(NULL, _T("인덱스를 찾을 수 없습니다"), _T("알림"), MB_ICONERROR);
	}
	udpSocketInfo* ptr = udpSocketInfoArray[nIndex];

	// 해당 인덱스 정보 삭제하기
	delete ptr;


	// 해당 인덱스에 배열 마지막 원소 넣어주기
	if (nIndex != (nTotalUdpSockets - 1))
		udpSocketInfoArray[nIndex] = udpSocketInfoArray[nTotalUdpSockets - 1];

	--nTotalUdpSockets;
}