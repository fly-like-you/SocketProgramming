#pragma once
#include<windows.h>
#include<stdio.h>
#include <tchar.h>	
#include<CommCtrl.h>

/*14장 예시 그대로 사용*/
#define SIZE_DAT 256 // 전송 패킷(헤더 + 데이터) 전체 크기 
#define TYPE_CHAT     1000              // 메시지 타입: 채팅
#define SERVERPORT 9000
#define BUFSIZE    256
#define MAX_NAME_SIZE 20
#define MAX_USER 64 //최대 소켓 연결 개수
#define MAX_FILTERWORD_SIZE 20 //금지단어 최대 길이
#define MAX_FILTERARRAY_SIZE 64 //금지단어 배열 최대길이
#define MAX_BLACK_USER 50 //블랙유저 최대 수


#define TYPE_UDP_CONNECTION_REQUEST 1006
#define TYPE_UDP_CONNECTION_SUCCESS 1007
#define TYPE_UDP_DISCONNECTION     1008
// 메시지 헤더 정의
#pragma pack(1)
typedef struct _WELCOME_MSG
{
	int  color;
	char nickname[MAX_NAME_SIZE];
} WELCOME_MSG;

typedef struct _HEAD_MSG
{
	char nickname[MAX_NAME_SIZE];
	int  type;    // 메시지 타입 - 채팅, 선, 도형 등
	int  length;  // 해당 메시지 구조체의 크기
	int  color;
} HEAD_MSG;


static char blackaddr[INET_ADDRSTRLEN];     //블랙리스트 추가 할 ip를 받는 배열 생성
static char tmpfilter[MAX_NAME_SIZE];		//필터목록에 추가할 단어를 받는 배열 생성

typedef struct _BLACK_INFO {                     //블랙리스트를 저장할 구조체 생성
	char addr[INET_ADDRSTRLEN];
}BLACK_INFO;

typedef struct _CHAT_MSG
{
	char msg[SIZE_DAT];
	char nickname[MAX_NAME_SIZE];
} CHAT_MSG;

typedef struct _SOCKETINFO
{
	SOCKET sock;
	bool   isTCP;
	bool   isUDP;
	char* buf;
	char   nickname[MAX_NAME_SIZE];
	int    recvbytes;
	int	   sendbytes;
	bool   recvflag;
	bool   sendflag;
	int    color;
} SOCKETINFO;
#pragma pack(0)
