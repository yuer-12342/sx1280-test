#ifndef RANGING_DEV_LOG_H
#define RANGING_DEV_LOG_H

#include <stdint.h>

/*
 * 标定/阶段 A：轮末快照，Keil Watch 读 g_RangingDevLog。
 * validity: 0=INV 1=OK 2=HOLD 3=RAW
 * gate:     0=OK 1=INV_STATUS 2=LOW_SAMPLES 3=BAD_RSSI 4=HIGH_SPREAD 5=JUMP
 */
typedef struct
{
    uint32_t seq;
    uint32_t round_index;
    uint8_t validity;
    uint8_t gate_reason;
    uint8_t gate_passed;
    uint8_t sample_count;
    float round_m;
    float pub_m;
    float distance_m;
    int8_t rssi_dbm;
    uint16_t handshake_tx;
    uint32_t age_ms;
} RangingDevLog_t;

extern volatile RangingDevLog_t g_RangingDevLog;

void RangingDevLogUpdate( void );

#endif /* RANGING_DEV_LOG_H */
