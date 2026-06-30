#include "gpio.h"
#include "sx1280.h"
#include "timer_hc32.h"

void HC32_Sx1280PortA_IRQHandler(void)
{
    if (GPIO_IntFlagGet(GPIOA, GPIO_PIN_15))
    {
        GPIO_IntFlagClear(GPIOA, GPIO_PIN_15);
        SX1280OnDioIrq();
    }
}

void HC32_Sx1280SysTick_Handler(void)
{
    TimerProcess();
}
