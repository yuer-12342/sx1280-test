#ifndef RANGING_FENCE_H
#define RANGING_FENCE_H

#include "demoRanging.h"

/* 门控：样本数 >= ceil(RngRequestCount/2)；RSSI / 离散度 / 跳变 / Rnd-Dist 偏差 */
#define FENCE_GATE_MIN_RSSI_DBM           (-92)     //RSSI最低值
#define FENCE_GATE_MAX_RAW_SPREAD_RATIO   0.50f     //hop离散度最大比例
#define FENCE_GATE_MAX_JUMP_UP_M          4.0f
#define FENCE_GATE_MAX_JUMP_DOWN_M        1.5f
#define FENCE_GATE_MAX_PUB_RND_DELTA_M    0.8f       //发布值与原始值最大偏差
#define FENCE_GATE_MAX_JUMP_M             FENCE_GATE_MAX_JUMP_UP_M    //滑动窗口大小

/* 轮间滤波：最近 N 轮通过门控的距离取中值 */
#define FENCE_FILTER_WINDOW               3u

typedef enum
{
    RNG_FENCE_GATE_OK = 0,         //通过
    RNG_FENCE_GATE_INVALID_STATUS,      //芯片状态无效
	  RNG_FENCE_GATE_LOW_SAMPLES,          //样本数不足
    RNG_FENCE_GATE_BAD_RSSI,             //RSSI太低
    RNG_FENCE_GATE_HIGH_SPREAD,           //离散度过大
    RNG_FENCE_GATE_JUMP,                   //跳变过大
    RNG_FENCE_GATE_PUB_RND_DELTA,          //与发布值偏差太大
} RangingFenceGateReason_t;

typedef struct
{
    double roundDistance;          //本轮原始距离值
    double filteredDistance;       //滤波后的距离，窗口内的中位数
    double publishedDistance;        //最终发布距离
    uint8_t sampleCount;              //本轮hop有效数
    int8_t rssiDbm;                  //本轮RSSI
    uint8_t gatePassed;             //门控是否通过
    uint8_t hasPublished;            //是否已经发布历史
    RangingFenceGateReason_t gateReason;        //拒绝原因
} RangingFenceOutput_t;

void RangingFenceInit( void );
void RangingFenceProcessRound( DemoResult_t *res, DemoSettings_t *cfg );
const RangingFenceOutput_t *RangingFenceGetOutput( void );

#endif /* RANGING_FENCE_H */
