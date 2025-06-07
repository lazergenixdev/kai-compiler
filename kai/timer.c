#include "timer.h"

#if defined(_WIN32)
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
#elif defined(__linux__) || defined(__APPLE__)
#include <sys/time.h>

void Timer_Init() {}

struct timespec start;
struct timespec end;

void Timer_Start(Timer* timer) {
	clock_gettime(CLOCK_MONOTONIC, &start);
}

double Timer_ElapsedSeconds(Timer* timer) {
	clock_gettime(CLOCK_MONOTONIC, &end);
	uint64_t duration_ns = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
	return (double)duration_ns / 1e9;
}

double Timer_ElapsedMilliseconds(Timer* timer) {
	clock_gettime(CLOCK_MONOTONIC, &end);
	uint64_t duration_ns = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
	return (double)duration_ns / 1e6;
}
#endif
