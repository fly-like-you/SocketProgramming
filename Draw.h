#define PI 3.141592

#include <math.h>


// 사각형 그리기 함수
void drawRectangle(const HDC& hDC, WPARAM& wParam, LPARAM& lParam)
{
	MoveToEx(hDC, LOWORD(wParam), HIWORD(wParam), NULL); // x0, y0
	LineTo(hDC, LOWORD(lParam), HIWORD(wParam)); // x1, y0
	LineTo(hDC, LOWORD(lParam), HIWORD(lParam)); // x1, y1
	LineTo(hDC, LOWORD(wParam), HIWORD(lParam)); // x0, y1
	LineTo(hDC, LOWORD(wParam), HIWORD(wParam)); // x0, y1
}

// 라디안 구하기
double getRadian(int num) {
	return num * (PI / 180);
}


// 원 그리기 함수
void drawCircle(const HDC& hDC, int radius, WPARAM& wParam)
{
	int tempX = radius + LOWORD(wParam);
	int tempY = HIWORD(wParam);
	MoveToEx(hDC, tempX, tempY, NULL);
	for (int angle = 0; angle <= 360; angle++)
	{
		tempX = (cos(getRadian(angle)) * radius) + LOWORD(wParam);
		tempY = (sin(getRadian(angle)) * radius) + HIWORD(wParam);
		// 구한 점과 오브젝트 포지션의 점을 이어주는 선을 그린다.
		LineTo(hDC, tempX, tempY);
	}
}

// 선그리기 함수
void drawLine(const HDC& hDC, WPARAM& wParam, LPARAM& lParam)
{
	MoveToEx(hDC, LOWORD(wParam), HIWORD(wParam), NULL);
	LineTo(hDC, LOWORD(lParam), HIWORD(lParam));
}