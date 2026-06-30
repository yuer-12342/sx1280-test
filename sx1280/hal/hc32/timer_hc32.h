#ifndef __TIMER_HC32_H__
#define __TIMER_HC32_H__

#include <stdint.h>

typedef void (*TimerCallback)(void);

void TimerInit(void);
void TimerStart(TimerCallback callback, uint32_t delay_ms);
void TimerCancelTimer(void);
void TimerProcess(void);
uint32_t TimerGetTickMs(void);

#endif
