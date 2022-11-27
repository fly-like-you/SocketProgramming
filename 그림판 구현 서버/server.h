#pragma once
#include<windows.h>
#include<stdio.h>
#include <tchar.h>	
#include<CommCtrl.h>

/*14장 예시 그대로 사용*/
#define SIZE_TOT 256 // 전송 패킷(헤더 + 데이터) 전체 크기 
#define SIZE_DAT (SIZE_TOT-sizeof(int))
#define TYPE_CHAT     1000              // 메시지 타입: 채팅
#define SERVERPORT 9000
#define BUFSIZE    256
#define MAX_NAME_SIZE 20
#define MAX_USER 64 //최대 소켓 연결 개수 ////박종혁

/* 박준호 추가*/
// 메시지 헤더 정의
#pragma pack(1)
typedef struct _WELCOME_MSG
{
	char nickname[MAX_NAME_SIZE];
} WELCOME_MSG;

typedef struct _HEAD_MSG
{
	int  type;    // 메시지 타입 - 채팅, 선, 도형 등
	int  length;  // 해당 메시지 구조체의 크기
} HEAD_MSG;


typedef struct _BLACK_IP {
	int type;
	char ip[SIZE_DAT];
}BLACK_IP;


typedef struct _CHAT_MSG
{
	int  type;
	char msg[SIZE_DAT];
	char nickname[MAX_NAME_SIZE];
} CHAT_MSG;

typedef struct _SOCKETINFO
{
	SOCKET sock;
	bool   isIPv6;
	bool   isUDP;
	char*  buf;
	char   nickname[MAX_NAME_SIZE];
	int    recvbytes;
	int	   sendbytes;
	bool   recvflag;
	bool   sendflag;
} SOCKETINFO;
#pragma pack(0)
