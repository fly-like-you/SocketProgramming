#include "socket.h" // 소켓 관련 전역 변수 및 함수 선언
#include "PrjClient.h" // 윈도우 관련 전역 변수 및 함수 선언, 메시지 타입 정의
#include "resource.h"
#include "../Draw.h" // 그림 그리기 함수 서넌

/***************************************FOR PRACTICE*********************************/
// 주석추가
void sendFixedMsg(int msg_type, int msg_size)
{
	int retval;
	g_headmsg.type = msg_type;
	g_headmsg.length = msg_size;
	retval = send(g_sock, (char*)&g_headmsg, sizeof(HEAD_MSG), 0);
	//("fixed msg send: %d\r\n", retval);
	if (retval == SOCKET_ERROR)
		DisplayText("erase failed please try again\r\n");

}
void err_display(const char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(0x09, SUBLANG_DEFAULT),
		(char *)&lpMsgBuf, 0, NULL);
	//printf("[%s] %s\n", msg, (char *)lpMsgBuf);
	DisplayText("[%s] %s\r\n", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 이벤트 생성(각각 신호, 비신호 상태)
	g_hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (g_hReadEvent == NULL) return 1;
	g_hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (g_hWriteEvent == NULL) return 1;

	// 전역 변수 초기화(일부)
	g_chatmsg.type = TYPE_CHAT;
	g_drawlinemsg.type = TYPE_DRAWLINE;
	g_drawlinemsg.color = RGB(255, 0, 0);
	g_erasepicmsg.type = TYPE_ERASEPIC;
	g_drawRectangle.type = TYPE_DRAWRECTANGLE; // 추가추가추가추가 사각형
	g_drawRectangle.color = RGB(0, 0, 0);
	g_drawCircle.type = TYPE_DRAWCIRCLE; // 추가추가추가추가 사각형
	g_drawCircle.color = RGB(0, 0, 0);
	// 대화상자 생성
	g_hInstance = hInstance;
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc); // 대화상자에서 발생하는 메시지는 맨 마지막 인수로 전달한 DlgProc()함수에서 처리한다

	// 이벤트 제거
	CloseHandle(g_hReadEvent);
	CloseHandle(g_hWriteEvent);

	return 0;
}

// 대화상자 프로시저
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int retval;					// 추가추가추가 박준호 

	static HWND hChkIsIPv6;
	static HWND hEditIPaddr;
	static HWND hEditPort;
	static HWND hChkIsUDP;
	static HWND hBtnConnect;
	static HWND hBtnSendFile; // 전역 변수에도 저장
	static HWND hBtnSendMsg; // 전역 변수에도 저장
	static HWND hEditMsg;
	static HWND hEditStatus; // 전역 변수에도 저장
	static HWND hColorRed;
	static HWND hBtnErasePic; // 전역 변수에도 저장
	static HWND hStaticDummy;
	static HWND hPencil;
	static HWND hEraser;
	// 박준호 추가 변수
	static HWND hDrawRectangle;
	static HWND hDrawCircle;
	// 닉네임 설정
	static HWND hEditNickName;
	// 닉네임 변경하는 버튼
	static HWND btnNickNameChange;

	switch (uMsg) {
	case WM_INITDIALOG:
		// 컨트롤 핸들 얻기
		hChkIsIPv6 = GetDlgItem(hDlg, IDC_ISIPV6);
		hEditIPaddr = GetDlgItem(hDlg, IDC_IPADDR);
		hEditPort = GetDlgItem(hDlg, IDC_PORT);
		hChkIsUDP = GetDlgItem(hDlg, IDC_ISUDP);
		hBtnConnect = GetDlgItem(hDlg, IDC_CONNECT);
		hBtnSendFile = GetDlgItem(hDlg, IDC_SENDFILE);
		g_hBtnSendFile = hBtnSendFile; // 전역 변수에 저장
		hBtnSendMsg = GetDlgItem(hDlg, IDC_SENDMSG);
		g_hBtnSendMsg = hBtnSendMsg; // 전역 변수에 저장
		hEditMsg = GetDlgItem(hDlg, IDC_MSG);
		hEditStatus = GetDlgItem(hDlg, IDC_STATUS);
		g_hEditStatus = hEditStatus; // 전역 변수에 저장
		hColorRed = GetDlgItem(hDlg, IDC_COLORRED);
		hBtnErasePic = GetDlgItem(hDlg, IDC_ERASEPIC);
		g_hBtnErasePic = hBtnErasePic; // 전역 변수에 저장
		hStaticDummy = GetDlgItem(hDlg, IDC_DUMMY);
		hPencil = GetDlgItem(hDlg, IDC_PENCIL);
		hEraser = GetDlgItem(hDlg, IDC_ERASER);
		//박준호 추가
		hDrawRectangle = GetDlgItem(hDlg, IDC_RECTANGLE);
		hDrawCircle = GetDlgItem(hDlg, IDC_CIRCLE);



		// 컨트롤 초기화
		SetDlgItemText(hDlg, IDC_IPADDR, SERVERIP4);
		SetDlgItemInt(hDlg, IDC_PORT, SERVERPORT, FALSE);
		EnableWindow(g_hBtnSendFile, FALSE);
		EnableWindow(g_hBtnSendMsg, FALSE);
		SendMessage(hEditMsg, EM_SETLIMITTEXT, SIZE_DAT / 2, 0);
		SendMessage(hColorRed, BM_SETCHECK, BST_CHECKED, 0);
		SendMessage(hPencil, BM_SETCHECK, BST_UNCHECKED, 0);
		SendMessage(hEraser, BM_SETCHECK, BST_UNCHECKED, 0);
		SendMessage(hDrawRectangle, BM_SETCHECK, BST_UNCHECKED, 0); // 박준호 추가추가추가
		SendMessage(hDrawCircle, BM_SETCHECK, BST_UNCHECKED, 0); // 박준호 추가추가추가
		EnableWindow(g_hBtnErasePic, FALSE);
		//닉네임 변경 버튼
		EnableWindow(btnNickNameChange, TRUE);
		EnableWindow(hEditNickName, TRUE);

		// 윈도우 클래스 등록
		WNDCLASS wndclass;
		wndclass.style = CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = ChildWndProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = g_hInstance;
		wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = _T("MyWndClass");
		if (!RegisterClass(&wndclass)) exit(1);

		// 자식 윈도우 생성
		RECT rect; GetWindowRect(hStaticDummy, &rect);
		POINT pt; pt.x = rect.left; pt.y = rect.top;
		ScreenToClient(hDlg, &pt);
		g_hDrawWnd = CreateWindow(_T("MyWndClass"), _T(""), WS_CHILD,
			pt.x, pt.y, rect.right - rect.left, rect.bottom - rect.top,
			hDlg, (HMENU)NULL, g_hInstance, NULL);
		if (g_hDrawWnd == NULL) exit(1);
		ShowWindow(g_hDrawWnd, SW_SHOW);
		UpdateWindow(g_hDrawWnd);

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CONNECT:
			// 컨트롤 상태 얻기
			if (strlen(g_welcome_msg.nickname) == 0) {
				MessageBox(hDlg, _T("닉네임 입력 후 연결 할 수 있습니다."), _T("경고"), MB_ICONERROR);
				return TRUE;
			}
			GetDlgItemTextA(hDlg, IDC_IPADDR, g_ipaddr, sizeof(g_ipaddr));
			GetDlgItemTextA(hDlg, IDC_NICKNAME, g_welcome_msg.nickname, SIZE_DAT);
			g_port = GetDlgItemInt(hDlg, IDC_PORT, NULL, TRUE);



			g_isUDP = SendMessage(hChkIsUDP, BM_GETCHECK, 0, 0);
			// 소켓 통신 스레드 시작
			g_hClientThread = CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);
			if (g_hClientThread == NULL) exit(0);
			// 서버 접속 성공 기다림
			while (g_bCommStarted == false);
			// 컨트롤 상태 변경
			EnableWindow(hChkIsIPv6, FALSE);
			EnableWindow(hEditIPaddr, FALSE);
			EnableWindow(hEditPort, FALSE);
			EnableWindow(hChkIsUDP, FALSE);
			EnableWindow(hBtnConnect, FALSE);
			EnableWindow(g_hBtnSendFile, TRUE);
			EnableWindow(g_hBtnSendMsg, TRUE);
			SetFocus(hEditMsg);
			EnableWindow(g_hBtnErasePic, TRUE);

			//닉네임 관련된 부분
			EnableWindow(btnNickNameChange, FALSE);
			EnableWindow(hEditNickName, FALSE);

			return TRUE;
		case IDC_NICKNAMECHANGEBTN:
			EnableWindow(g_hBtnSendMsg, FALSE);
			GetDlgItemTextA(hDlg, IDC_NICKNAME, g_welcome_msg.nickname, SIZE_DAT);

			if (strlen(g_welcome_msg.nickname) == 0) {
				MessageBox(hDlg, _T("닉네임은 공백이 될 수 없습니다"), _T("경고"), MB_ICONERROR);
				return true;
			}

			//send(g_sock, (char*)&g_chatmsg, SIZE_TOT, 0);

			Sleep(100);

			EnableWindow(hEditNickName, FALSE);
			EnableWindow(btnNickNameChange, FALSE);

			MessageBox(NULL, _T("닉네임이 정상적으로 설정되었습니다."), _T("성공"), MB_OK);

			return TRUE;
		case IDC_SENDFILE:
			MessageBox(NULL, _T("아직 구현하지 않았습니다."), _T("알림"), MB_ICONERROR);
			// 1. GetOpenFileName() 윈도우 API를 이용해서 파일 열기 대화상자를 열고
			// 전송할 파일을 선택한다. 2. 선택한 파일을 읽어서 서버에 전송한다.
			return TRUE;
		case IDC_SENDMSG:

			// 이전에 얻은 채팅 메시지 읽기 완료를 기다림
			WaitForSingleObject(g_hReadEvent, INFINITE);
			// 새로운 채팅 메시지를 얻고 쓰기 완료를 알림
			GetDlgItemTextA(hDlg, IDC_MSG, g_chatmsg.msg, SIZE_DAT);
			SetEvent(g_hWriteEvent);
			// 입력된 텍스트 전체를 선택 표시
			SendMessage(hEditMsg, EM_SETSEL, 0, -1);
			return TRUE;
			// 닉네임 변경버튼

		case IDC_PENCIL:
			g_drawlinemsg.color = RGB(255, 0, 0);
			return TRUE;

		case IDC_ERASER:
			g_drawlinemsg.color = RGB(255, 255, 255);
			return TRUE;

		case IDC_RECTANGLE:
			g_drawlinemsg.color = RGB(0, 0, 255);
			return TRUE;

		case IDC_CIRCLE:
			g_drawlinemsg.color = RGB(0, 255, 0);
			return TRUE;
		case IDC_ERASEPIC:
			if (MessageBox(hDlg, _T("정말로 지우시겠습니까?"),
				_T("질문"), MB_YESNO | MB_ICONQUESTION) == IDYES)
			{
				sendFixedMsg(TYPE_ERASEPIC, sizeof(g_erasepicmsg));
				send(g_sock, (char*)&g_erasepicmsg, sizeof(CIRCLE_MSG), 0);

			}
			return TRUE;
		case IDCANCEL:
			if (MessageBox(hDlg, _T("정말로 종료하시겠습니까?"),
				_T("질문"), MB_YESNO | MB_ICONQUESTION) == IDYES)
			{
				//send(g_sock, final_key, sizeof(int));
				closesocket(g_sock);
				EndDialog(hDlg, IDCANCEL);
			}
			return TRUE;
		}
	}
	return FALSE;
}



// 자식 윈도우 프로시저
LRESULT CALLBACK ChildWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hDC;
	HPEN hPen, hOldPen;
	PAINTSTRUCT ps;
	static int cx, cy;
	static HBITMAP hBitmap;
	static HDC hDCMem;
	static int x0, y0;
	static int x1, y1;
	static bool bDrawing;

	// 박준호 변수
	static int radius;
	switch (uMsg) {
	case WM_SIZE:
		// 화면 출력용 DC 핸들 얻기
		hDC = GetDC(hWnd);
		// 배경 비트맵과 메모리 DC 생성
		cx = LOWORD(lParam);
		cy = HIWORD(lParam);
		hBitmap = CreateCompatibleBitmap(hDC, cx, cy);
		hDCMem = CreateCompatibleDC(hDC);
		SelectObject(hDCMem, hBitmap);
		// 배경 비트맵 흰색으로 채움
		SelectObject(hDCMem, GetStockObject(WHITE_BRUSH));
		SelectObject(hDCMem, GetStockObject(WHITE_PEN));
		Rectangle(hDCMem, 0, 0, cx, cy);
		// 화면 출력용 DC 핸들 해제
		ReleaseDC(hWnd, hDC);
		return 0;
	case WM_PAINT: // 윈도우의 화면 전체를 갱신해야할 때
		// 화면 출력용 DC 핸들 얻기
		hDC = BeginPaint(hWnd, &ps);
		// 배경 비트맵을 화면에 전송
		BitBlt(hDC, 0, 0, cx, cy, hDCMem, 0, 0, SRCCOPY);
		// 화면 출력용 DC 핸들 해제
		EndPaint(hWnd, &ps);
		return 0;
	case WM_LBUTTONDOWN:
		// 마우스 클릭 좌표 얻기
		x0 = LOWORD(lParam);
		y0 = HIWORD(lParam);
		bDrawing = true;
		if (g_isRect) {
			// 사각형 그리기 메시지 보내기
			g_drawRectangle.x0 = x0;
			g_drawRectangle.y0 = y0;
		}
		else if (g_isCircle) {
			// 원 그리기 메시지 보내기
			g_drawCircle.x0 = x0;
			g_drawCircle.y0 = y0;
		}
		return 0;
	case WM_MOUSEMOVE:
		if (bDrawing && g_bCommStarted) {
			// 마우스 클릭 좌표 얻기
			x1 = LOWORD(lParam);
			y1 = HIWORD(lParam);

			if (!g_isRect && !g_isCircle) {
				// 선 그리기 메시지 보내기
				g_drawlinemsg.x0 = x0;
				g_drawlinemsg.y0 = y0;
				g_drawlinemsg.x1 = x1;
				g_drawlinemsg.y1 = y1;

				sendFixedMsg(TYPE_DRAWLINE, sizeof(g_drawlinemsg));
				send(g_sock, (char*)&g_drawlinemsg, sizeof(DRAWLINE_MSG), 0);

			}
			// 마우스 클릭 좌표 갱신
			x0 = x1;
			y0 = y1;
		}
		return 0;
	case WM_LBUTTONUP:
		bDrawing = false;
		if (g_isRect) {
			g_drawRectangle.x1 = x1;
			g_drawRectangle.y1 = y1;
			sendFixedMsg(TYPE_DRAWRECTANGLE, sizeof(g_drawRectangle));
			send(g_sock, (char*)&g_drawRectangle, sizeof(RECTANGLE_MSG), 0);
		}
		else if (g_isCircle) {
			g_drawCircle.x1 = x1;
			g_drawCircle.y1 = y1;
			sendFixedMsg(TYPE_DRAWCIRCLE, sizeof(g_drawCircle));
			send(g_sock, (char*)&g_drawCircle, sizeof(CIRCLE_MSG), 0);
		}
		return 0;
	case WM_DRAWLINE:
		// 화면 출력용 DC와 Pen 핸들 얻기
		hDC = GetDC(hWnd);
		hPen = CreatePen(PS_SOLID, 3, g_drawcolor);

		// 윈도우 화면에 1차로 그리기
		hOldPen = (HPEN)SelectObject(hDC, hPen);
		drawLine(hDC, wParam, lParam);
		SelectObject(hDC, hOldPen);

		// 배경 비트맵에 2차로 그리기
		hOldPen = (HPEN)SelectObject(hDCMem, hPen);
		drawLine(hDCMem, wParam, lParam);
		SelectObject(hDCMem, hOldPen);

		// 화면 출력용 DC와 Pen 핸들 해제
		DeleteObject(hPen);
		ReleaseDC(hWnd, hDC);
		return 0;

	case WM_DRAWRECT: // 추가추가추가
		// 화면 출력용 DC와 Pen 핸들 얻기
		hDC = GetDC(hWnd);
		hPen = CreatePen(PS_SOLID, 3, g_drawcolor);

		// 윈도우 화면에 1차로 그리기
		hOldPen = (HPEN)SelectObject(hDC, hPen);
		if (!bDrawing) {
			drawRectangle(hDC, wParam, lParam);
		}
		SelectObject(hDC, hOldPen);
		// 배경 비트맵에 2차로 그리기
		hOldPen = (HPEN)SelectObject(hDCMem, hPen);
		if (!bDrawing) {
			drawRectangle(hDCMem, wParam, lParam);
		}
		SelectObject(hDCMem, hOldPen);
		// 화면 출력용 DC와 Pen 핸들 해제
		DeleteObject(hPen);
		ReleaseDC(hWnd, hDC);
		return 0;

	case WM_DRAWCIRCLE:
		hDC = GetDC(hWnd);
		hPen = CreatePen(PS_SOLID, 3, g_drawcolor);
		// 윈도우 화면에 1차로 그리기
		hOldPen = (HPEN)SelectObject(hDC, hPen);
		radius = (int)sqrt(pow(LOWORD(lParam) - LOWORD(wParam), 2) + pow(HIWORD(lParam) - HIWORD(wParam), 2));
		if (!bDrawing) {
			drawCircle(hDC, radius, wParam);
		}
		SelectObject(hDC, hOldPen);


		// 배경 비트맵에 2차로 그리기
		hOldPen = (HPEN)SelectObject(hDCMem, hPen);
		if (!bDrawing) {
			drawCircle(hDCMem, radius, wParam);
		}
		SelectObject(hDCMem, hOldPen);
		// 화면 출력용 DC와 Pen 핸들 해제
		DeleteObject(hPen);
		ReleaseDC(hWnd, hDC);
		return 0;

	case WM_ERASEPIC:

		// 배경 비트맵 흰색으로 채움
		SelectObject(hDCMem, GetStockObject(WHITE_BRUSH));
		SelectObject(hDCMem, GetStockObject(WHITE_PEN));
		Rectangle(hDCMem, 0, 0, cx, cy);
		// WM_PAINT 메시지 강제 생성
		InvalidateRect(hWnd, NULL, FALSE);

		return 0;
	case WM_DESTROY:
		DeleteDC(hDCMem);
		DeleteObject(hBitmap);
		PostQuitMessage(0);
		
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}




//소켓 통신 스레드 함수(1) - 메인
DWORD WINAPI ClientMain(LPVOID arg)
{
	int retval;


	if (g_isUDP == false) { // TCP/IPv4 서버 
		g_sock = newSocketConnect(SOCK_STREAM); // 소켓 생성 및 커넥트
	}
	else if (g_isUDP == true) { // UDP/IPv4 서버
		g_sock = newSocketConnect(SOCK_DGRAM);
	}
	// 클라이언트 최초 통신
	send(g_sock, (char*)&g_welcome_msg, sizeof(WELCOME_MSG), 0);
	
	MessageBox(NULL, _T("서버에 접속했습니다."), _T("알림"), MB_ICONINFORMATION);

	// 읽기 & 쓰기 스레드 생성
	HANDLE hThread[2];
	hThread[0] = CreateThread(NULL, 0, WriteThread, NULL, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, ReadThread, NULL, 0, NULL);
	if (hThread[0] == NULL || hThread[1] == NULL) exit(1);
	g_bCommStarted = true;

	// 스레드 종료 대기 (둘 중 하나라도 종료할 때까지)
	retval = WaitForMultipleObjects(2, hThread, FALSE, INFINITE);
	retval -= WAIT_OBJECT_0;
	if (retval == 0)
		TerminateThread(hThread[1], 1);
	else
		TerminateThread(hThread[0], 1);
	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);

	MessageBox(NULL, _T("연결이 끊겼습니다."), _T("알림"), MB_ICONERROR);

	EnableWindow(g_hBtnSendFile, FALSE);
	EnableWindow(g_hBtnSendMsg, FALSE);
	EnableWindow(g_hBtnErasePic, FALSE);
	g_bCommStarted = false;
	closesocket(g_sock);
	return 0;
}

// 소켓 통신 스레드 함수(2) - 데이터 수신
DWORD WINAPI ReadThread(LPVOID arg)
{
	int retval;
	CHAT_MSG chat_msg;
	DRAWLINE_MSG drawline_msg;
	ERASEPIC_MSG erasepic_msg;
	RECTANGLE_MSG drawrect_msg;
	CIRCLE_MSG drawcircle_msg;

	// 추가추가추가
	int msg_type;
	int msg_length;
	HEAD_MSG head_msg;

	while (1) {
		// 박준호 추가추가 고정 크기 데이터 받기
		retval = recv(g_sock, (char*)&head_msg, sizeof(HEAD_MSG), MSG_WAITALL);
		//DisplayText("retval: %d len: %d\r\n", retval, sizeof(HEAD_MSG));
		if (retval == SOCKET_ERROR) {
			err_quit("recv()");
		}

		msg_length = head_msg.length;
		msg_type = head_msg.type;


		// 크기와 타입을 받기
		

		if (msg_type == TYPE_CHAT) {
			retval = recv(g_sock, (char*)&chat_msg, msg_length, MSG_WAITALL);
			DisplayText("[%s's message]  %s\r\n", chat_msg.nickname, chat_msg.msg); //닉네임 보냄
		}
		else if (msg_type == TYPE_DRAWLINE) {
			//drawline_msg = (DRAWLINE_MSG*)&comm_msg;
			recv(g_sock, (char*)&drawline_msg, msg_length, MSG_WAITALL);
			g_drawcolor = drawline_msg.color;

			SendMessage(g_hDrawWnd, WM_DRAWLINE,
				MAKEWPARAM(drawline_msg.x0, drawline_msg.y0),
				MAKELPARAM(drawline_msg.x1, drawline_msg.y1));
		}
		else if (msg_type == TYPE_ERASEPIC) {
			retval= recv(g_sock, (char*)&erasepic_msg, msg_length, MSG_WAITALL);
			//DisplayText("variable msg recv %d\r\n", retval);
			if (retval == 0 || retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			SendMessage(g_hDrawWnd, WM_ERASEPIC, 0, 0);
		}
		else if (msg_type == TYPE_DRAWRECTANGLE) {
			recv(g_sock, (char*)&drawrect_msg, msg_length, MSG_WAITALL);

			g_drawcolor = drawrect_msg.color;
			SendMessage(g_hDrawWnd, WM_DRAWRECT,
				MAKEWPARAM(drawrect_msg.x0, drawrect_msg.y0),
				MAKELPARAM(drawrect_msg.x1, drawrect_msg.y1)
			);
		}
		else if (msg_type == TYPE_DRAWCIRCLE) {
			recv(g_sock, (char*)&drawcircle_msg, msg_length, MSG_WAITALL);
			g_drawcolor = drawcircle_msg.color;
			SendMessage(g_hDrawWnd, WM_DRAWCIRCLE,
				MAKEWPARAM(drawcircle_msg.x0, drawcircle_msg.y0),
				MAKELPARAM(drawcircle_msg.x1, drawcircle_msg.y1)
			);
		}

	}
	return 0;
}

// 소켓 통신 스레드 함수(3) - 데이터 송신
DWORD WINAPI WriteThread(LPVOID arg)
{
	int retval;
	HEAD_MSG head_msg;
	
	// 서버와 데이터 통신
	while (1) {
		// 쓰기 완료 기다리기
		WaitForSingleObject(g_hWriteEvent, INFINITE);

		// 문자열 길이가 0이면 보내지 않음
		if (strlen(g_chatmsg.msg) == 0) {
			// [메시지 전송] 버튼 활성화
			EnableWindow(g_hBtnSendMsg, TRUE);
			// 읽기 완료 알리기
			SetEvent(g_hReadEvent);
			continue;
		}
		// 추가 고정 크기 + 고정 길이
		// 메시지의 길이 

		head_msg.length = sizeof(g_chatmsg);
		head_msg.type = TYPE_CHAT;
		strcpy(g_chatmsg.nickname, g_welcome_msg.nickname);
		retval = send(g_sock, (char*)&head_msg, sizeof(HEAD_MSG), 0);
		//DisplayText("send -> retval: %d len: %d\r\n", retval, sizeof(HEAD_MSG));
		// 데이터 보내기 + 가변 길이
		retval = send(g_sock, (char*)&g_chatmsg, head_msg.length, 0);
		//DisplayText("send -> retval: %d len: %d\r\n", retval, head_msg.length);

		if (retval == SOCKET_ERROR) break;
		// [메시지 전송] 버튼 활성화
		EnableWindow(g_hBtnSendMsg, TRUE);
		// 읽기 완료 알리기
		SetEvent(g_hReadEvent);
	}
	return 0;
}

// 에디트 컨트롤 출력 함수
void DisplayText(const char* fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	char cbuf[1024];
	vsprintf(cbuf, fmt, arg);
	va_end(arg);

	int nLength = GetWindowTextLength(g_hEditStatus);
	SendMessage(g_hEditStatus, EM_SETSEL, nLength, nLength);
	SendMessageA(g_hEditStatus, EM_REPLACESEL, FALSE, (LPARAM)cbuf);
}
