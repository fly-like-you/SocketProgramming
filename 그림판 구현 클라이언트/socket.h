#pragma once
#include "..\Common.h"

/* 통신 관련 전역 변수 */
static volatile bool g_isIPv6;        // IPv4 or IPv6 주소?
static char          g_ipaddr[64];    // 서버 IP 주소(문자열)
static int           g_port;          // 서버 포트 번호
static volatile bool g_isUDP;         // TCP or UDP 프로토콜?
static HANDLE        g_hClientThread; // 스레드 핸들
static volatile bool g_bCommStarted;  // 통신 시작 여부
static SOCKET        g_sock;          // 클라이언트 소켓
static HANDLE        g_hReadEvent;    // 이벤트 핸들(1)
static HANDLE        g_hWriteEvent;   // 이벤트 핸들(2)


SOCKET newSocketConnect(int socket_type) // 인수를 받아서 udp, tcp소켓 생성
{
	int retval;
	SOCKET sock;
	sock = socket(AF_INET, socket_type, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, g_ipaddr, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(g_port);
	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");
	return sock;
}

