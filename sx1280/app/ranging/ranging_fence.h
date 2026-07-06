#ifndef RANGING_FENCE_H
#define RANGING_FENCE_H

#include "demoRanging.h"

/* L1 硬门控 */
#define FENCE_GATE_MIN_RSSI_DBM           (-92)
#define FENCE_GATE_MAX_JUMP_DOWN_M        1.5f

/* L2 稳态门控 */
#define FENCE_GATE_MAX_RAW_SPREAD_RATIO   0.50f
#define FENCE_GATE_SPREAD_RATIO_FAR       0.65f
#define FENCE_GATE_SPREAD_FAR_THRESH_M    4.0f
#define FENCE_GATE_STATIC_DELTA_M         0.8f

#define FENCE_GATE_MAX_JUMP_UP_M          4.0f
#define FENCE_GATE_MAX_PUB_RND_DELTA_M    FENCE_GATE_STATIC_DELTA_M
#define FENCE_GATE_MAX_JUMP_M             FENCE_GATE_MAX_JUMP_UP_M

/* L3 趋势跟踪：识别真实移动 vs 异变 */
#define FENCE_TREND_WINDOW                3u
#define FENCE_TRACK_CONFIRM_ROUNDS        2u
#define FENCE_TRACK_RND_SPREAD_MAX_M      1.2f
#define FENCE_TRACK_RND_SPREAD_FAR_M      1.8f
#define FENCE_TRACK_MIN_DELTA_M           0.4f
#define FENCE_TRACK_CREEP_FAR_MAX_M       1.5f
#define FENCE_TRACK_CREEP_NEAR_MAX_M      1.0f
#define FENCE_TRACK_MONO_TOL_M            0.3f

#define FENCE_FILTER_WINDOW               3u

typedef enum
{
    RNG_FENCE_GATE_OK = 0,
    RNG_FENCE_GATE_INVALID_STATUS,
    RNG_FENCE_GATE_LOW_SAMPLES,
    RNG_FENCE_GATE_BAD_RSSI,
    RNG_FENCE_GATE_HIGH_SPREAD,
    RNG_FENCE_GATE_JUMP,
    RNG_FENCE_GATE_PUB_RND_DELTA,
    RNG_FENCE_GATE_TRACK,
    RNG_FENCE_GATE_ANOMALY,
    RNG_FENCE_GATE_PENDING,
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
