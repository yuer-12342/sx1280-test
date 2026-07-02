#ifndef FENCE_ALARM_H
#define FENCE_ALARM_H

#include <stdint.h>

/*
 * 室内粗测：在 main.c 中 #define FENCE_ALARM_INDOOR_DEMO 1（于 #include "fence_alarm.h" 之前）
 * Set 默认 1 m，预设 1 / 1.5 / 2 / 3 m，滞回 0.5 m。
 */
#ifndef FENCE_ALARM_INDOOR_DEMO
#define FENCE_ALARM_INDOOR_DEMO  0
#endif

typedef enum
{
    FENCE_ALARM_STATE_SAFE = 0,
    FENCE_ALARM_STATE_OUT,
} FenceAlarmState_t;

void FenceAlarmInit( void );
void FenceAlarmPollInput( void );
void FenceAlarmOnRound( void );
void FenceAlarmPollOutput( void );

float FenceAlarmGetThresholdM( void );
uint8_t FenceAlarmGetPresetIndex( void );
uint8_t FenceAlarmGetPresetCount( void );
FenceAlarmState_t FenceAlarmGetState( void );
uint8_t FenceAlarmIsEnabled( void );

/* 按键导致 Set/Alm 变化后置 1，显示刷新后清零 */
uint8_t FenceAlarmTakeUiDirty( void );

#endif /* FENCE_ALARM_H */
