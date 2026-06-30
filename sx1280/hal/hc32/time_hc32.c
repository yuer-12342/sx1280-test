#include "timer_hc32.h"
#include "ddl.h"
#include <stddef.h>

static void (*activeCallback)(void) = NULL;
static volatile uint32_t remainingTicks = 0;
static volatile uint32_t s_elapsedMs = 0u;

void TimerInit(void)
{
    if (SysTick_Config(SystemCoreClock / 1000))
    {
        while (1) {}
    }
    NVIC_SetPriority(SysTick_IRQn, 1);
}

void TimerStart(TimerCallback callback, uint32_t delay_ms)
{
    activeCallback = callback;
    remainingTicks = delay_ms;
}

void TimerCancelTimer(void)
{
    activeCallback = NULL;
    remainingTicks = 0;
}

void TimerProcess(void)
{
    s_elapsedMs++;

    if (activeCallback != NULL && remainingTicks > 0)
    {
        remainingTicks--;
        if (remainingTicks == 0)
        {
            void (*cb)(void) = activeCallback;
            activeCallback = NULL;
            if (cb != NULL)
            {
                cb();
            }
        }
    }
}

uint32_t TimerGetTickMs(void)
{
    return s_elapsedMs;
}
