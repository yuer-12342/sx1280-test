#include "ranging_dev_log.h"
#include "ranging_publish.h"

volatile RangingDevLog_t g_RangingDevLog;

void RangingDevLogUpdate( void )
{
    const RangingPublish_t *pub = RangingPublishGet();

    g_RangingDevLog.seq++;
    g_RangingDevLog.round_index   = pub->round_index;
    g_RangingDevLog.validity      = (uint8_t)pub->validity;
    g_RangingDevLog.gate_reason   = (uint8_t)pub->gate_reason;
    g_RangingDevLog.gate_passed   = pub->gate_passed;
    g_RangingDevLog.sample_count  = pub->sample_count;
    g_RangingDevLog.round_m       = pub->round_distance_m;
    g_RangingDevLog.pub_m         = pub->published_distance_m;
    g_RangingDevLog.distance_m    = pub->distance_m;
    g_RangingDevLog.rssi_dbm      = pub->rssi_dbm;
    g_RangingDevLog.handshake_tx  = pub->handshake_tx;
    g_RangingDevLog.age_ms        = pub->age_ms;
}
