#include "hw.h"
#include "sysctrl.h"
#include "sx1280.h"
#include "timer_hc32.h"
#include "system_hc32l021.h"

void HwInit(void)
{
    stc_sysctrl_clock_init_t clockInit;

    SYSCTRL_ClockStcInit(&clockInit);
    clockInit.u32SysClockSrc = SYSCTRL_CLK_SRC_RC48M_6M;
    clockInit.u32HclkDiv = SYSCTRL_HCLK_PRS_DIV1;
    SYSCTRL_ClockInit(&clockInit);
    SystemCoreClockUpdate();

    TimerInit();

    {
        DioIrqHandler *irqHandlers[3] = { SX1280OnDioIrq, NULL, NULL };
        SX1280HalInit(irqHandlers);
    }

    SX1280SetPollingMode();
}
