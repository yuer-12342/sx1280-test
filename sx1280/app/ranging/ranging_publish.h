#ifndef RANGING_PUBLISH_H
#define RANGING_PUBLISH_H

#include <stdint.h>
#include "demoRanging.h"
#include "ranging_fence.h"

/*!
 * \brief 对外发布有效性（Step-2 统一语义）
 *
 * OK    : L2 稳态通过，distance_m = 滤波发布值
 * TRACK : L3 趋势限速更新，distance_m = 跟踪后发布值
 * HOLD  : 未通过或累积中，distance_m = 上次发布值
 * RAW  : 尚无历史发布，distance_m = 本轮中值（预热）
 * INV  : 无可用距离
 */
typedef enum
{
    RNG_PUBLISH_INVALID = 0,
    RNG_PUBLISH_OK,
    RNG_PUBLISH_TRACK,
    RNG_PUBLISH_HOLD,
    RNG_PUBLISH_RAW,
} RangingPublishValidity_t;

typedef struct
{
    float distance_m;
    float round_distance_m;
    float published_distance_m;
    RangingPublishValidity_t validity;
    RangingFenceGateReason_t gate_reason;
    uint8_t gate_passed;
    uint8_t sample_count;
    int8_t rssi_dbm;
    uint16_t handshake_tx;
    uint32_t age_ms;
    uint32_t round_index;
} RangingPublish_t;

void RangingPublishInit( void );
void RangingPublishUpdateFromRound( DemoResult_t *res, DemoSettings_t *cfg );
const RangingPublish_t *RangingPublishGet( void );

#endif /* RANGING_PUBLISH_H */
