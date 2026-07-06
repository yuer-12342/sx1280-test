#ifndef RANGING_DEV_LOG_H
#define RANGING_DEV_LOG_H

#include <stdint.h>

/*
 * 标定/阶段 A：轮末快照，Keil Watch 读 g_RangingDevLog。
 * validity: 0=INV 1=OK 2=TRACK 3=HOLD 4=RAW
 * gate:     0=OK 1=INV 2=LOW 3=RSSI 4=SPREAD 5=JUMP 6=dRND 7=TRK 8=ANO 9=WAIT
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
