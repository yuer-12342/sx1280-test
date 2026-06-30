#include "hw-lptim.h"
#include "timer_hc32.h"

void TimerCreateTimer(void *func, void *param, uint32_t millisec)
{
    (void)param;
    TimerStart((TimerCallback)func, millisec);
}
