#include "ranging_publish.h"
#include "timer_hc32.h"

static RangingPublish_t s_publish;
static uint32_t s_lastOkTickMs;

static uint16_t RangingPublishGetHandshakeTx( DemoResult_t *res )
{
    if( ( res->CntPacketTx == 0u ) && ( res->CntPacketRxOK > 0u ) )
    {
        return 1u;
    }

    return (uint16_t)res->CntPacketTx;
}

void RangingPublishInit( void )
{
    s_publish.distance_m           = 0.0f;
    s_publish.round_distance_m       = 0.0f;
    s_publish.published_distance_m   = 0.0f;
    s_publish.validity               = RNG_PUBLISH_INVALID;
    s_publish.gate_reason            = RNG_FENCE_GATE_OK;
    s_publish.gate_passed            = 0u;
    s_publish.sample_count           = 0u;
    s_publish.rssi_dbm               = 0;
    s_publish.handshake_tx           = 0u;
    s_publish.age_ms                 = 0u;
    s_publish.round_index            = 0u;
    s_lastOkTickMs                   = 0u;
}

void RangingPublishUpdateFromRound( DemoResult_t *res, DemoSettings_t *cfg )
{
    const RangingFenceOutput_t *fence = RangingFenceGetOutput();
    uint32_t now = TimerGetTickMs();

    s_publish.round_index++;
    s_publish.round_distance_m     = (float)res->RngDistance;
    s_publish.published_distance_m = (float)fence->publishedDistance;
    s_publish.gate_reason          = fence->gateReason;
    s_publish.gate_passed          = fence->gatePassed;
    s_publish.sample_count         = fence->sampleCount;
    s_publish.rssi_dbm             = fence->rssiDbm;
    s_publish.handshake_tx         = RangingPublishGetHandshakeTx( res );

    if( fence->gatePassed != 0u )
    {
        s_publish.distance_m = (float)fence->publishedDistance;
        if( fence->gateReason == RNG_FENCE_GATE_TRACK )
        {
            s_publish.validity = RNG_PUBLISH_TRACK;
        }
        else
        {
            s_publish.validity = RNG_PUBLISH_OK;
        }
        s_lastOkTickMs   = now;
        s_publish.age_ms = 0u;
    }
    else if( fence->hasPublished != 0u )
    {
        s_publish.distance_m = (float)fence->publishedDistance;   //�þ�ֵ
        s_publish.validity   = RNG_PUBLISH_HOLD;                 //��ͨ������ʷ
        if( s_lastOkTickMs != 0u )
        {
            s_publish.age_ms = now - s_lastOkTickMs;
        }
        else
        {
            s_publish.age_ms = 0u;
        }
    }
    else if( ( cfg->RngStatus == RNG_VALID ) && ( res->RngDistance > 0.0 ) )
    {
        s_publish.distance_m = (float)res->RngDistance;
        s_publish.validity   = RNG_PUBLISH_RAW;
        s_publish.age_ms     = 0u;
    }
    else
    {
        s_publish.distance_m = 0.0f;
        s_publish.validity   = RNG_PUBLISH_INVALID;
        s_publish.age_ms     = 0u;
    }
}

const RangingPublish_t *RangingPublishGet( void )
{
    return &s_publish;
}
