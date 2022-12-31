#pragma once

BOOL BlockingUser(struct sockaddr_in sockaddr) {
	unsigned short port;
	char ipAddress[INET_ADDRSTRLEN];
	char int2stringPort[6] = { 0. };

	// 사용자 ip 주소 받아오기
	getIpPort(ipAddress, &port, &sockaddr);
	for (int i = 0; i < TotalBlackUser; i++) {  // 블랙리스트 배열과 비교해서
		if (!strcmp(ipAddress, g_blackinfo[i].addr)) {  // 배열이 존재하면
			MessageBoxA(NULL, (const char*)"허가되지 않은 사용자 1명이 차단되었습니다.", (const char*)"Administrator", MB_ICONERROR);
			return TRUE;  // TRUE 리턴
		}
	}
	// 배열이 존재하지 않으면 
	// FAlSE 리턴
	return FALSE;
}


// 소켓 정보 추가
int AddUdpSocketInfo(struct sockaddr_in sockaddr, char* nickname)
{
	if (nTotalUdpSockets >= FD_SETSIZE) {
		printf("[오류] 소켓 정보를 추가할 수 없습니다!\n");
		return -1;
	}

	udpSocketInfo* ptr = new udpSocketInfo;
	if (ptr == NULL) {
		printf("[오류] 메모리가 부족합니다!\n");
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
void RemoveUdpSocketInfo(struct sockaddr_in* sockaddr)
{
	// 인덱스 찾기
	if (BlockingUser(*sockaddr)) {
		return;
	}
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