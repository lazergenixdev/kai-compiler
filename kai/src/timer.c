#include "timer.h"

#ifdef _WIN32
#include <Windows.h>

static LARGE_INTEGER frequency;

void Timer_Init() {
	QueryPerformanceFrequency(&frequency);
}

void Timer_Start(Timer* timer) {
	QueryPerformanceCounter((LARGE_INTEGER*)timer);
}

double Timer_ElapsedSeconds(Timer* timer) {
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);
	LONGLONG duration = end.QuadPart - ((LARGE_INTEGER*)timer)->QuadPart;
	return (double)(duration) / (double)(frequency.QuadPart);
}

double Timer_ElapsedMilliseconds(Timer* timer) {
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);
	LONGLONG duration = end.QuadPart - ((LARGE_INTEGER*)timer)->QuadPart;
	return (double)(duration * 1000) / (double)(frequency.QuadPart);
}

#endif // _WIN32
