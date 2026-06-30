#ifndef HW_LPTIM_H
#define HW_LPTIM_H

#include <stdint.h>

void TimerCreateTimer(void *func, void *param, uint32_t millisec);
void TimerCancelTimer(void);

#endif
