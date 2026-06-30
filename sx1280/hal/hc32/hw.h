#ifndef HW_H
#define HW_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "ddl.h"
#include "hw-lptim.h"
#include "sx1280_hal_hc32.h"
#include "board_hc32l021.h"

void HwInit(void);

#define HAL_Delay(ms)  DDL_Delay1ms(ms)

#endif
