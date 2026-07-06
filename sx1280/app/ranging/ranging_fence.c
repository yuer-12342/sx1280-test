#include "ranging_fence.h"

static RangingFenceOutput_t s_fence;
static double s_filterWindow[FENCE_FILTER_WINDOW];
static uint8_t s_filterCount;
static uint8_t s_filterWriteIdx;

static int8_t RangingFencePickRssi( DemoResult_t *res )     //RSSI信号检查
{
    int8_t rssi;
    uint16_t i;

    if( res->RngResultIndex > 0u )
    {
        for( i = ( uint16_t )res->RngResultIndex; i > 0u; --i )
        {
            rssi = res->RawRngRssi[i - 1u];
            if( rssi != 0 )
            {
                return rssi;
            }
        }
    }

    return res->RssiValue;
}

static double RangingFenceMedian( double *values, uint8_t count )   //中位数计算
{
    double sorted[FENCE_FILTER_WINDOW];
    uint8_t i;
    uint8_t k;
    uint8_t pass;

    if( count == 0u )
    {
        return 0.0;
    }

    for( i = 0u; i < count; ++i )
    {
        sorted[i] = values[i];
    }

    for( pass = 0u; pass < ( count - 1u ); ++pass )    //冒泡排序取中位数
    {
        for( k = 0u; k < ( count - 1u - pass ); ++k )
        {
            if( sorted[k] > sorted[k + 1u] )          
            {
                double temp = sorted[k];
                sorted[k] = sorted[k + 1u];
                sorted[k + 1u] = temp;
            }
        }
    }

    if( ( count % 2u ) == 0u )        //如果是偶数个则取中间两个平均值，奇数个数据则取中间那个
    {
        return ( sorted[count / 2u - 1u] + sorted[count / 2u] ) * 0.5;
    }

    return sorted[count / 2u];
}

static uint8_t RangingFenceCheckRawSpread( DemoResult_t *res )
{
    double minVal;
    double maxVal;
    double median;
    double spread;
    uint16_t i;

    if( res->RngResultIndex < 3 )   //只有达到3个Hop才能计算离散度
    {
        return 1u;                   //样本数太少
    }

    minVal = res->RngResults[0];
    maxVal = res->RngResults[0];
    for( i = 1u; i < ( uint16_t )res->RngResultIndex; ++i )
    {
        if( res->RngResults[i] < minVal )
        {
            minVal = res->RngResults[i];
        }
        if( res->RngResults[i] > maxVal )
        {
            maxVal = res->RngResults[i];
        }
    }

    median = ( maxVal + minVal ) * 0.5;    //最大+最小/2  ，作为中位数近似
    if( median <= 0.0 )
    {
        return 1u;
    }

    spread = ( maxVal - minVal ) / median;      //离散度计算
    if( spread > ( double )FENCE_GATE_MAX_RAW_SPREAD_RATIO )
    {
        return 0u;
    }

    return 1u;
}

static uint8_t RangingFenceGetMinSamples( DemoSettings_t *cfg )
{
    if( cfg->RngRequestCount <= 1u )
    {
        return 1u;
    }

    return ( uint8_t )( ( cfg->RngRequestCount + 1u ) / 2u );
}

static RangingFenceGateReason_t RangingFenceEvaluateGate( DemoResult_t *res, DemoSettings_t *cfg )
{
    int8_t rssi;
    uint8_t minSamples;

    if( cfg->RngStatus != RNG_VALID )   //芯片状态检查
    {
        return RNG_FENCE_GATE_INVALID_STATUS;
    }

    if( res->RngDistance <= 0.0 )      //距离有效性检查
    {
        return RNG_FENCE_GATE_INVALID_STATUS;
    }

    minSamples = RangingFenceGetMinSamples( cfg );      //样本数检查
    if( ( uint8_t )res->RngResultIndex < minSamples )
    {
        return RNG_FENCE_GATE_LOW_SAMPLES;
    }

    rssi = RangingFencePickRssi( res );               //RSSI检查
    if( ( rssi != 0 ) && ( rssi < FENCE_GATE_MIN_RSSI_DBM ) )
    {
        return RNG_FENCE_GATE_BAD_RSSI;
    }

    if( RangingFenceCheckRawSpread( res ) == 0u )       //hop离散度检查
    {
        return RNG_FENCE_GATE_HIGH_SPREAD;
    }

    if( ( s_fence.hasPublished != 0u ) &&       //跳变检查，向上为4米，向下1.5米
        ( res->RngDistance > 0.0 ) &&
        ( s_fence.publishedDistance > 0.0 ) )
    {
        double delta = res->RngDistance - s_fence.publishedDistance;
        double absDelta;

        if( delta > ( double )FENCE_GATE_MAX_JUMP_UP_M )
        {
            return RNG_FENCE_GATE_JUMP;
        }
        if( delta < -( double )FENCE_GATE_MAX_JUMP_DOWN_M )
        {
            return RNG_FENCE_GATE_JUMP;
        }

        absDelta = delta;
        if( absDelta < 0.0 )       //偏差大于0.8米也拒绝
        {
            absDelta = -absDelta;
        }
        if( absDelta > ( double )FENCE_GATE_MAX_PUB_RND_DELTA_M )
        {
            return RNG_FENCE_GATE_PUB_RND_DELTA;
        }
    }

    return RNG_FENCE_GATE_OK;
}

static void RangingFencePushFilter( double distance )         //循环缓冲区
{
    s_filterWindow[s_filterWriteIdx] = distance;             //存入新值
    s_filterWriteIdx = ( uint8_t )( ( s_filterWriteIdx + 1u ) % FENCE_FILTER_WINDOW );    //移动指针
    if( s_filterCount < FENCE_FILTER_WINDOW )
    {
        s_filterCount++;                                   //未填满则加1
    }
}

void RangingFenceInit( void )      //围栏初始化
{
    uint8_t i;

    s_fence.roundDistance = 0.0;
    s_fence.filteredDistance = 0.0;
    s_fence.publishedDistance = 0.0;
    s_fence.sampleCount = 0u;
    s_fence.rssiDbm = 0;
    s_fence.gatePassed = 0u;
    s_fence.hasPublished = 0u;
    s_fence.gateReason = RNG_FENCE_GATE_OK;

    s_filterCount = 0u;
    s_filterWriteIdx = 0u;
    for( i = 0u; i < FENCE_FILTER_WINDOW; ++i )
    {
        s_filterWindow[i] = 0.0;
    }
}

void RangingFenceProcessRound( DemoResult_t *res, DemoSettings_t *cfg )  
{
    RangingFenceGateReason_t reason;     //记录的本轮原始数据

    s_fence.roundDistance = res->RngDistance;
    s_fence.sampleCount = ( uint8_t )res->RngResultIndex;
    s_fence.rssiDbm = RangingFencePickRssi( res );  
    s_fence.gatePassed = 0u;

    reason = RangingFenceEvaluateGate( res, cfg );    //执行门控检查
    s_fence.gateReason = reason;

    if( reason != RNG_FENCE_GATE_OK )            //不通过直接返回。publishedDistance不变
    {
        return;
    }

    s_fence.gatePassed = 1u;              //通过、压入窗口、取中位数、更新发布值
    RangingFencePushFilter( res->RngDistance );  //
    s_fence.filteredDistance = RangingFenceMedian( s_filterWindow, s_filterCount );
    s_fence.publishedDistance = s_fence.filteredDistance;
    s_fence.hasPublished = 1u;
}

const RangingFenceOutput_t *RangingFenceGetOutput( void )
{
    return &s_fence;
}
