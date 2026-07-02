#include "ranging_fence.h"

static RangingFenceOutput_t s_fence;
static double s_filterWindow[FENCE_FILTER_WINDOW];
static uint8_t s_filterCount;
static uint8_t s_filterWriteIdx;

static int8_t RangingFencePickRssi( DemoResult_t *res )
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

static double RangingFenceMedian( double *values, uint8_t count )
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

    for( pass = 0u; pass < ( count - 1u ); ++pass )
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

    if( ( count % 2u ) == 0u )
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

    if( res->RngResultIndex < 3 )
    {
        return 1u;
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

    median = ( maxVal + minVal ) * 0.5;
    if( median <= 0.0 )
    {
        return 1u;
    }

    spread = ( maxVal - minVal ) / median;
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

    if( cfg->RngStatus != RNG_VALID )
    {
        return RNG_FENCE_GATE_INVALID_STATUS;
    }

    if( res->RngDistance <= 0.0 )
    {
        return RNG_FENCE_GATE_INVALID_STATUS;
    }

    minSamples = RangingFenceGetMinSamples( cfg );
    if( ( uint8_t )res->RngResultIndex < minSamples )
    {
        return RNG_FENCE_GATE_LOW_SAMPLES;
    }

    rssi = RangingFencePickRssi( res );
    if( ( rssi != 0 ) && ( rssi < FENCE_GATE_MIN_RSSI_DBM ) )
    {
        return RNG_FENCE_GATE_BAD_RSSI;
    }

    if( RangingFenceCheckRawSpread( res ) == 0u )
    {
        return RNG_FENCE_GATE_HIGH_SPREAD;
    }

    if( ( s_fence.hasPublished != 0u ) &&
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
        if( absDelta < 0.0 )
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

static void RangingFencePushFilter( double distance )
{
    s_filterWindow[s_filterWriteIdx] = distance;
    s_filterWriteIdx = ( uint8_t )( ( s_filterWriteIdx + 1u ) % FENCE_FILTER_WINDOW );
    if( s_filterCount < FENCE_FILTER_WINDOW )
    {
        s_filterCount++;
    }
}

void RangingFenceInit( void )
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
    RangingFenceGateReason_t reason;

    s_fence.roundDistance = res->RngDistance;
    s_fence.sampleCount = ( uint8_t )res->RngResultIndex;
    s_fence.rssiDbm = RangingFencePickRssi( res );
    s_fence.gatePassed = 0u;

    reason = RangingFenceEvaluateGate( res, cfg );
    s_fence.gateReason = reason;

    if( reason != RNG_FENCE_GATE_OK )
    {
        return;
    }

    s_fence.gatePassed = 1u;
    RangingFencePushFilter( res->RngDistance );  //
    s_fence.filteredDistance = RangingFenceMedian( s_filterWindow, s_filterCount );
    s_fence.publishedDistance = s_fence.filteredDistance;
    s_fence.hasPublished = 1u;
}

const RangingFenceOutput_t *RangingFenceGetOutput( void )
{
    return &s_fence;
}
