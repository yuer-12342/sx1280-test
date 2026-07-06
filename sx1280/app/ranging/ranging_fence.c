#include "ranging_fence.h"

static RangingFenceOutput_t s_fence;
static double s_filterWindow[FENCE_FILTER_WINDOW];
static uint8_t s_filterCount;
static uint8_t s_filterWriteIdx;

static double s_trendRnd[FENCE_TREND_WINDOW];
static uint8_t s_trendCount;

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

static float RangingFenceGetSpreadRatioLimit( double roundDist )
{
    if( roundDist > ( double )FENCE_GATE_SPREAD_FAR_THRESH_M )
    {
        return FENCE_GATE_SPREAD_RATIO_FAR;
    }

    return FENCE_GATE_MAX_RAW_SPREAD_RATIO;
}

static uint8_t RangingFenceCheckRawSpread( DemoResult_t *res )
{
    double minVal;
    double maxVal;
    double median;
    double spread;
    float limit;
    uint16_t i;

    if( res->RngResultIndex < 3u )
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

    limit  = RangingFenceGetSpreadRatioLimit( res->RngDistance );
    spread = ( maxVal - minVal ) / median;
    if( spread > ( double )limit )
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

static RangingFenceGateReason_t RangingFenceCheckHard( DemoResult_t *res, DemoSettings_t *cfg )
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

    if( ( s_fence.hasPublished != 0u ) &&
        ( s_trendCount < FENCE_TRACK_CONFIRM_ROUNDS ) &&
        ( res->RngDistance < s_fence.publishedDistance - ( double )FENCE_GATE_MAX_JUMP_DOWN_M ) )
    {
        return RNG_FENCE_GATE_JUMP;
    }

    return RNG_FENCE_GATE_OK;
}

static void RangingFenceTrendClear( void )
{
    uint8_t i;

    s_trendCount = 0u;
    for( i = 0u; i < FENCE_TREND_WINDOW; ++i )
    {
        s_trendRnd[i] = 0.0;
    }
}

static void RangingFenceTrendPush( double rnd )
{
    if( s_trendCount < FENCE_TREND_WINDOW )
    {
        s_trendRnd[s_trendCount] = rnd;
        s_trendCount++;
    }
    else
    {
        uint8_t i;

        for( i = 1u; i < FENCE_TREND_WINDOW; ++i )
        {
            s_trendRnd[i - 1u] = s_trendRnd[i];
        }
        s_trendRnd[FENCE_TREND_WINDOW - 1u] = rnd;
    }
}

static void RangingFenceTrendStats( double *median, double *spread )
{
    double minVal;
    double maxVal;
    uint8_t i;

    *median = RangingFenceMedian( s_trendRnd, s_trendCount );
    minVal  = s_trendRnd[0];
    maxVal  = s_trendRnd[0];
    for( i = 1u; i < s_trendCount; ++i )
    {
        if( s_trendRnd[i] < minVal )
        {
            minVal = s_trendRnd[i];
        }
        if( s_trendRnd[i] > maxVal )
        {
            maxVal = s_trendRnd[i];
        }
    }
    *spread = maxVal - minVal;
}

static uint8_t RangingFenceTrendMonotonicAway( void )
{
    uint8_t i;

    if( s_trendCount < 2u )
    {
        return 0u;
    }

    for( i = 1u; i < s_trendCount; ++i )
    {
        if( s_trendRnd[i] + ( double )FENCE_TRACK_MONO_TOL_M < s_trendRnd[i - 1u] )
        {
            return 0u;
        }
    }

    return 1u;
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

static void RangingFencePublishOk( double distance )
{
    RangingFenceTrendClear();
    RangingFencePushFilter( distance );
    s_fence.filteredDistance  = RangingFenceMedian( s_filterWindow, s_filterCount );
    s_fence.publishedDistance = s_fence.filteredDistance;
    s_fence.hasPublished      = 1u;
    s_fence.gatePassed        = 1u;
    s_fence.gateReason        = RNG_FENCE_GATE_OK;
}

static void RangingFencePublishTrack( double trendMedian, double published )
{
    double delta;
    double step;
    double next;

    delta = trendMedian - published;
    if( delta > 0.0 )
    {
        step = delta;
        if( step > ( double )FENCE_TRACK_CREEP_FAR_MAX_M )
        {
            step = ( double )FENCE_TRACK_CREEP_FAR_MAX_M;
        }
    }
    else
    {
        step = delta;
        if( step < -( double )FENCE_TRACK_CREEP_NEAR_MAX_M )
        {
            step = -( double )FENCE_TRACK_CREEP_NEAR_MAX_M;
        }
    }

    next = published + step;
    RangingFenceTrendClear();
    RangingFencePushFilter( next );
    s_fence.filteredDistance  = RangingFenceMedian( s_filterWindow, s_filterCount );
    s_fence.publishedDistance = s_fence.filteredDistance;
    s_fence.hasPublished      = 1u;
    s_fence.gatePassed        = 1u;
    s_fence.gateReason        = RNG_FENCE_GATE_TRACK;
}

static void RangingFenceTryTrendTrack( void )
{
    double trendMedian;
    double trendSpread;
    double delta;
    double spreadLimit;

    if( s_trendCount < FENCE_TRACK_CONFIRM_ROUNDS )
    {
        s_fence.gatePassed = 0u;
        s_fence.gateReason = RNG_FENCE_GATE_PENDING;
        return;
    }

    RangingFenceTrendStats( &trendMedian, &trendSpread );

    spreadLimit = ( double )FENCE_TRACK_RND_SPREAD_MAX_M;
    if( RangingFenceTrendMonotonicAway() != 0u )
    {
        spreadLimit = ( double )FENCE_TRACK_RND_SPREAD_FAR_M;
    }

    if( trendSpread > spreadLimit )
    {
        RangingFenceTrendClear();
        s_fence.gatePassed = 0u;
        s_fence.gateReason = RNG_FENCE_GATE_ANOMALY;
        return;
    }

    if( s_fence.hasPublished == 0u )
    {
        RangingFencePublishOk( trendMedian );
        return;
    }

    delta = trendMedian - s_fence.publishedDistance;
    if( delta < 0.0 )
    {
        delta = -delta;
    }
    if( delta < ( double )FENCE_TRACK_MIN_DELTA_M )
    {
        s_fence.gatePassed = 0u;
        s_fence.gateReason = RNG_FENCE_GATE_PENDING;
        return;
    }

    RangingFencePublishTrack( trendMedian, s_fence.publishedDistance );
}

void RangingFenceInit( void )
{
    uint8_t i;

    s_fence.roundDistance     = 0.0;
    s_fence.filteredDistance  = 0.0;
    s_fence.publishedDistance = 0.0;
    s_fence.sampleCount       = 0u;
    s_fence.rssiDbm           = 0;
    s_fence.gatePassed        = 0u;
    s_fence.hasPublished      = 0u;
    s_fence.gateReason        = RNG_FENCE_GATE_OK;

    s_filterCount    = 0u;
    s_filterWriteIdx = 0u;
    for( i = 0u; i < FENCE_FILTER_WINDOW; ++i )
    {
        s_filterWindow[i] = 0.0;
    }

    RangingFenceTrendClear();
}

void RangingFenceProcessRound( DemoResult_t *res, DemoSettings_t *cfg )
{
    RangingFenceGateReason_t hard;
    double delta;
    double absDelta;
    uint8_t spreadOk;

    s_fence.roundDistance = res->RngDistance;
    s_fence.sampleCount   = ( uint8_t )res->RngResultIndex;
    s_fence.rssiDbm       = RangingFencePickRssi( res );
    s_fence.gatePassed    = 0u;

    hard = RangingFenceCheckHard( res, cfg );
    if( hard != RNG_FENCE_GATE_OK )
    {
        RangingFenceTrendClear();
        s_fence.gateReason = hard;
        return;
    }

    spreadOk = RangingFenceCheckRawSpread( res );

    if( spreadOk != 0u )
    {
        if( s_fence.hasPublished == 0u )
        {
            RangingFencePublishOk( res->RngDistance );
            return;
        }

        delta = res->RngDistance - s_fence.publishedDistance;
        absDelta = delta;
        if( absDelta < 0.0 )
        {
            absDelta = -absDelta;
        }

        if( absDelta <= ( double )FENCE_GATE_STATIC_DELTA_M )
        {
            RangingFencePublishOk( res->RngDistance );
            return;
        }
    }

    RangingFenceTrendPush( res->RngDistance );
    RangingFenceTryTrendTrack();
}

const RangingFenceOutput_t *RangingFenceGetOutput( void )
{
    return &s_fence;
}
