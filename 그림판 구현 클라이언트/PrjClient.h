#pragma once

#define SERVERIP4  _T("127.0.0.1")
#define SERVERPORT  9000

#define SIZE_DAT 256                    // 전송 패킷(헤더 + 데이터) 전체 크기


#define MAX_NAME_SIZE 20				//닉네임 최대 길이

#define TYPE_CHAT     1000              // 메시지 타입: 채팅
#define TYPE_DRAWLINE 1001              // 메시지 타입: 선 그리기
#define TYPE_ERASEPIC 1002              // 메시지 타입: 그림 지우기
#define TYPE_DRAWRECTANGLE 1003			// 메시지 타입: 사각형 그리기
#define TYPE_DRAWCIRCLE 1004			// 메시지 타입: 원 그리기
#define TYPE_ERASEPEN 1005

/* UDP 전용 타입 */
#define TYPE_UDP_CONNECTION_REQUEST 1006		// UDP 사용자: 접속 요청
#define TYPE_UDP_CONNECTION_SUCCESS 1007		// UDP 사용자: 접속 성공
#define TYPE_UDP_DISCONNECTION     1008         // UDP 사용자: 접속 종료



#define WM_DRAWLINE (WM_USER+1)         // 사용자 정의 메시지: 직선 메시지
#define WM_ERASEPIC (WM_USER+2)         // 사용자 정의 메시지: 모두 지우기 메시지
#define WM_DRAWRECT (WM_USER+3)         // 사용자 정의 메시지: 사각형 메시지
#define WM_DRAWCIRCLE (WM_USER+4)       // 사용자 정의 메시지: 원 메시지
#define WM_ERASEPEN (WM_USER+5)			// 사용자 정의 메시지: 지우개 펜 메시지

// 메시지 헤더 정의
#pragma pack(1)

// TCP 전송 메시지
// 최초 접속시 주고 받는 메시지
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



// 채팅 메시지 형식
typedef struct _CHAT_MSG
{
	char msg[SIZE_DAT];
	char nickname[MAX_NAME_SIZE];
} CHAT_MSG;

// 선 그리기 메시지 형식
typedef struct _DRAWLINE_MSG
{
	int  x0, y0;
	int  x1, y1;
} DRAWLINE_MSG;

// 그림 지우기 메시지 형식
typedef struct _ERASEPIC_MSG
{
	int  type;
} ERASEPIC_MSG;

// 사각형 그리기 메시지 형식
typedef struct _RECTANGLE_MSG
{
	int x0, y0;
	int x1, y1;
}RECTANGLE_MSG;


// 원 그리기 메시지 형식
typedef struct _CIRCLE_MSG
{
	int x0, y0;
	int x1, y1;
}CIRCLE_MSG;

// 지우개 펜 메시지 형식
typedef struct _ERASEPEN_MSG
{
	int  x0, y0;
	int  x1, y1;
} ERASEPEN_MSG;
#pragma pack(0)


/* 윈도우 관련 전역 변수 */
static HINSTANCE     g_hInstance;     // 프로그램 인스턴스 핸들
static HWND          g_hBtnSendMsg;   // [메시지 전송] 버튼
static HWND          g_hEditStatus;   // 각종 메시지 출력 영역
static HWND          g_hBtnErasePic;  // [그림 지우기] 버튼
static HWND          g_hDrawWnd;      // 그림을 그릴 윈도우



/* 메시지 관련 전역 변수 */
static CHAT_MSG      g_chatmsg;       // 채팅 메시지
static DRAWLINE_MSG  g_drawlinemsg;   // 선 그리기 메시지
static ERASEPEN_MSG  g_erasepenmsg;   // 선 그리기 메시지
static int           g_drawcolor;     // 선 그리기 색상
static ERASEPIC_MSG  g_erasepicmsg;   // 그림 지우기 메시지


static RECTANGLE_MSG g_drawRectangle; //  사각형 그리기 메시지
static CIRCLE_MSG    g_drawCircle;    //  원 그리기메시지
static HEAD_MSG      g_headmsg;       // 고정 크기 메시지  헤더
static WELCOME_MSG   g_welcome_msg;   // 웰컴 메시지

static int           g_isPen;		  // 펜 토글
static int           g_isRect;        //  사각형 토글
static int			 g_isCircle;	  //  원 토글
static int           g_isErasePen;    //  지우개 토글



// 윈도우 함수


// 대화상자 프로시저
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
void sendFixedVariableMsg();
// 자식 윈도우 프로시저
LRESULT CALLBACK ChildWndProc(HWND, UINT, WPARAM, LPARAM);
void drawLine(const HDC& hDC, WPARAM& wParam, LPARAM& lParam);
void drawCircle(int radius, WPARAM& wParam, const HDC& hDC);
void drawRectangle2(const HDC& hDC, WPARAM& wParam, LPARAM& lParam);
void drawRectangle(const HDC& hDC, WPARAM& wParam, LPARAM& lParam);
void newSocketConnect(int& retval);
// 소켓 통신 스레드 함수
DWORD WINAPI ClientMain(LPVOID arg);
DWORD WINAPI ReadThread(LPVOID arg);
DWORD WINAPI WriteThread(LPVOID arg);
// 에디트 컨트롤 출력 함수
void DisplayText(const char* fmt, ...);


