#ifndef RANGING_FENCE_H
#define RANGING_FENCE_H

#include "demoRanging.h"

/* 门控：样本数 >= ceil(RngRequestCount/2)；RSSI / 离散度 / 跳变 */
#define FENCE_GATE_MIN_RSSI_DBM           (-92)
#define FENCE_GATE_MAX_RAW_SPREAD_RATIO   0.50f
#define FENCE_GATE_MAX_JUMP_M             4.0

/* 轮间滤波：最近 N 轮通过门控的距离取中值 */
#define FENCE_FILTER_WINDOW               3u

typedef enum
{
    RNG_FENCE_GATE_OK = 0,
    RNG_FENCE_GATE_INVALID_STATUS,
    RNG_FENCE_GATE_LOW_SAMPLES,
    RNG_FENCE_GATE_BAD_RSSI,
    RNG_FENCE_GATE_HIGH_SPREAD,
    RNG_FENCE_GATE_JUMP,
} RangingFenceGateReason_t;

typedef struct
{
    double roundDistance;
    double filteredDistance;
    double publishedDistance;
    uint8_t sampleCount;
    int8_t rssiDbm;
    uint8_t gatePassed;
    uint8_t hasPublished;
    RangingFenceGateReason_t gateReason;
} RangingFenceOutput_t;

void RangingFenceInit( void );
void RangingFenceProcessRound( DemoResult_t *res, DemoSettings_t *cfg );
const RangingFenceOutput_t *RangingFenceGetOutput( void );

#endif /* RANGING_FENCE_H */
