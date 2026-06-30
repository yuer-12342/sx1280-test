#ifndef __SX1280_HAL_H__
#define __SX1280_HAL_H__

/*
 * Semtech sx1280-hal.h 的 HC32 适配：仅转发到平台 HAL，不修改 sx1280.c
 */
#include "sx1280_hal_hc32.h"

#ifndef RADIO_DIO1_ENABLE
#define RADIO_DIO1_ENABLE  1
#endif
#ifndef RADIO_DIO2_ENABLE
#define RADIO_DIO2_ENABLE  0
#endif
#ifndef RADIO_DIO3_ENABLE
#define RADIO_DIO3_ENABLE  0
#endif

#endif
