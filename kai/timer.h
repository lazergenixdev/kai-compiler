#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>

typedef uintptr_t Timer;

extern void   Timer_Init();
extern void   Timer_Start(Timer* timer);
extern double Timer_ElapsedSeconds(Timer* timer);
extern double Timer_ElapsedMilliseconds(Timer* timer);

#endif // TIMER_H