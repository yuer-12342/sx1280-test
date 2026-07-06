#include "ranging_display.h"
#include "lcd.h"
#include "demoRanging.h"
#include "ranging_fence.h"
#include "ranging_publish.h"
#include "fence_alarm.h"

#define RNG_DISPLAY_HOLD    250u
#define RNG_DISPLAY_RAW     251u
#define RNG_DISPLAY_TRACK   252u

/*
 * 80x160 标定屏布局（12px 字，行距 16）
 * Stat/Dist/Rnd/Hop/Gate/RSSI/No/Htx — 均来自 RangingPublish（轮末更新）
 */
#define COL_LBL_X           2u
#define COL_VAL_X           38u
#define ROW_Y_STAT          18u
#define ROW_Y_DIST          34u
#define ROW_Y_RND           50u
#define ROW_Y_HOP           66u
#define ROW_Y_GATE          82u
#define ROW_Y_RSSI          98u
#define ROW_Y_SET           114u
#define ROW_Y_ALM           130u

static const u8 *RangingDisplayStatusText(uint8_t status)
{
    switch (status)
    {
    case RNG_PROCESS:
        return (const u8 *)"Run";
    case RNG_VALID:
        return (const u8 *)"Done";
    case RNG_TIMEOUT:
        return (const u8 *)"TOut";
    case RNG_PER_ERROR:
        return (const u8 *)"Err";
    case RNG_DISPLAY_RAW:
        return (const u8 *)"Raw";
    case RNG_DISPLAY_TRACK:
        return (const u8 *)"Trk";
    case RNG_INIT:
    default:
        if (status == RNG_DISPLAY_HOLD)
        {
            return (const u8 *)"Hold";
        }
        return (const u8 *)"Init";
    }
}

static void RangingDisplayShowU16(u16 x, u16 y, uint16_t num, u8 width, u16 fc, u16 bc)
{
    u8 i;
    u16 div = 1u;
    u8 w;

    for (w = 1u; w < width; w++)
    {
        div = (u16)( div * 10u );
    }

    for (i = 0u; i < width; i++)
    {
        u8 digit = (u8)((num / div) % 10u);
        LCD_ShowChar((u16)(x + i * 6u), y, (u8)('0' + digit), fc, bc, 12, 0);
        div /= 10u;
    }
}

static void RangingDisplayShowDistance(u16 x, u16 y, float dist, u16 fc, u16 bc)
{
    u16 ipart = (u16)dist;
    u16 fpart = (u16)((dist - (float)ipart) * 100.0f + 0.5f);

    if (fpart >= 100u)
    {
        fpart = 99u;
    }

    if (ipart >= 10u)
    {
        LCD_ShowChar(x, y, (u8)('0' + (ipart / 10u) % 10u), fc, bc, 12, 0);
        LCD_ShowChar((u16)(x + 6u), y, (u8)('0' + ipart % 10u), fc, bc, 12, 0);
        LCD_ShowChar((u16)(x + 12u), y, '.', fc, bc, 12, 0);
        LCD_ShowChar((u16)(x + 18u), y, (u8)('0' + fpart / 10u), fc, bc, 12, 0);
        LCD_ShowChar((u16)(x + 24u), y, (u8)('0' + fpart % 10u), fc, bc, 12, 0);
    }
    else
    {
        LCD_ShowChar(x, y, (u8)('0' + ipart), fc, bc, 12, 0);
        LCD_ShowChar((u16)(x + 6u), y, '.', fc, bc, 12, 0);
        LCD_ShowChar((u16)(x + 12u), y, (u8)('0' + fpart / 10u), fc, bc, 12, 0);
        LCD_ShowChar((u16)(x + 18u), y, (u8)('0' + fpart % 10u), fc, bc, 12, 0);
    }
}

#define RSSI_FIELD_WIDTH    5u

static void RangingDisplayShowRssi(u16 x, u16 y, int8_t rssi, u16 fc, u16 bc)
{
    u8 buf[RSSI_FIELD_WIDTH + 1u];
    u8 i;
    u8 pos;
    u16 mag;

    for (i = 0u; i < RSSI_FIELD_WIDTH; i++)
    {
        buf[i] = (u8)' ';
    }

    if (rssi == 0)
    {
        buf[0] = '-';
        buf[1] = '-';
        buf[2] = '-';
    }
    else
    {
        pos = 0u;
        if (rssi < 0)
        {
            buf[pos++] = '-';
            mag = (u16)( -(int16_t)rssi );
        }
        else
        {
            mag = (u16)rssi;
        }

        if (mag >= 100u)
        {
            buf[pos++] = (u8)('0' + (mag / 100u));
            mag %= 100u;
        }
        if ( ( mag >= 10u ) || ( pos > ( ( rssi < 0 ) ? 1u : 0u ) ) )
        {
            buf[pos++] = (u8)('0' + (mag / 10u));
            mag %= 10u;
        }
        buf[pos++] = (u8)('0' + mag);
    }

    buf[RSSI_FIELD_WIDTH] = 0u;
    LCD_ShowString(x, y, buf, fc, bc, 12, 0);
}

static void RangingDisplayShowStatusFixed(u16 x, u16 y, const u8 *text, u16 fc, u16 bc)
{
    u8 buf[5];
    u8 i;

    for (i = 0; i < 4u; i++)
    {
        buf[i] = (text[i] != 0u) ? text[i] : (u8)' ';
    }
    buf[4] = 0u;
    LCD_ShowString(x, y, buf, fc, bc, 12, 0);
}

static const u8 *RangingDisplayGateText( RangingFenceGateReason_t reason )
{
    switch( reason )
    {
    case RNG_FENCE_GATE_OK:
        return (const u8 *)"OK  ";
    case RNG_FENCE_GATE_INVALID_STATUS:
        return (const u8 *)"InvS";
    case RNG_FENCE_GATE_LOW_SAMPLES:
        return (const u8 *)"LowS";
    case RNG_FENCE_GATE_BAD_RSSI:
        return (const u8 *)"Rssi";
    case RNG_FENCE_GATE_HIGH_SPREAD:
        return (const u8 *)"Sprd";
    case RNG_FENCE_GATE_JUMP:
        return (const u8 *)"Jump";
    case RNG_FENCE_GATE_PUB_RND_DELTA:
        return (const u8 *)"dRnd";
    case RNG_FENCE_GATE_TRACK:
        return (const u8 *)"Trk ";
    case RNG_FENCE_GATE_ANOMALY:
        return (const u8 *)"Ano ";
    case RNG_FENCE_GATE_PENDING:
        return (const u8 *)"Wait";
    default:
        return (const u8 *)"??? ";
    }
}

static void RangingDisplayShowGate( u16 x, u16 y, RangingFenceGateReason_t reason, u16 fc, u16 bc )
{
    RangingDisplayShowStatusFixed( x, y, RangingDisplayGateText( reason ), fc, bc );
}

static void RangingDisplayShowDashDist(u16 x, u16 y, u16 fc, u16 bc)
{
    LCD_ShowString(x, y, (const u8 *)"--.--", fc, bc, 12, 0);
}

static void RangingDisplayShowDashField(u16 x, u16 y, u16 fc, u16 bc)
{
    LCD_ShowString(x, y, (const u8 *)"-----", fc, bc, 12, 0);
}

static uint8_t RangingDisplayResolveStatus(uint8_t status, DemoResult_t *res)
{
    const RangingFenceOutput_t *fence = RangingFenceGetOutput();
    const RangingPublish_t *pub = RangingPublishGet();

    if( ( status == RNG_INIT ) || ( status == RNG_PROCESS ) || ( status == RNG_TIMEOUT ) )
    {
        if( RangingDemoIsContinuousMode() != 0u )
        {
            if( ( res->CntPacketTx == 0u ) && ( fence->hasPublished == 0u ) && ( res->RngDistance <= 0.0 ) )
            {
                return RNG_INIT;
            }

            return RNG_PROCESS;
        }

        if( ( status == RNG_INIT || status == RNG_TIMEOUT ) && ( res->CntPacketRxOK > 0u ) )
        {
            return RNG_PROCESS;
        }

        return status;
    }

    if( pub->round_index > 0u )
    {
        if( pub->validity == RNG_PUBLISH_HOLD )
        {
            return RNG_DISPLAY_HOLD;
        }
        if( pub->validity == RNG_PUBLISH_TRACK )
        {
            return RNG_DISPLAY_TRACK;
        }
        if( pub->validity == RNG_PUBLISH_RAW )
        {
            return RNG_DISPLAY_RAW;
        }
        if( pub->validity == RNG_PUBLISH_OK )
        {
            return RNG_VALID;
        }
        if( pub->validity == RNG_PUBLISH_INVALID )
        {
            return RNG_PER_ERROR;
        }
    }

    if( ( status == RNG_VALID ) && ( res->RngDistance > 0.0 ) )
    {
        return RNG_VALID;
    }

    if( status == RNG_PER_ERROR )
    {
        return RNG_PER_ERROR;
    }

    if( status == RNG_TIMEOUT )
    {
        return RNG_TIMEOUT;
    }

    if( fence->hasPublished != 0u )
    {
        return RNG_VALID;
    }

    return RNG_PER_ERROR;
}

uint8_t RangingDisplayCanRefresh(void)
{
    return RangingDemoDisplayUpdateAllowed() ? 1u : 0u;
}

static void RangingDisplayShowAlmState(u16 x, u16 y, u16 fc, u16 bc)
{
    if( FenceAlarmIsEnabled() == 0u )
    {
        RangingDisplayShowStatusFixed( x, y, (const u8 *)"Mute", GRAY, bc );
        return;
    }

    if( FenceAlarmGetState() == FENCE_ALARM_STATE_OUT )
    {
        RangingDisplayShowStatusFixed( x, y, (const u8 *)"OUT!", RED, bc );
    }
    else
    {
        RangingDisplayShowStatusFixed( x, y, (const u8 *)"Safe", GREEN, bc );
    }
}

static void RangingDisplayShowRunPlaceholders(void)
{
    RangingDisplayShowDashDist(COL_VAL_X, ROW_Y_DIST, GRAY, BLACK);
    RangingDisplayShowDashDist(COL_VAL_X, ROW_Y_RND, GRAY, BLACK);
    RangingDisplayShowDashField(COL_VAL_X, ROW_Y_HOP, GRAY, BLACK);
    RangingDisplayShowDashField(COL_VAL_X, ROW_Y_GATE, GRAY, BLACK);
    RangingDisplayShowRssi(COL_VAL_X, ROW_Y_RSSI, 0, GRAY, BLACK);
    RangingDisplayShowDistance(COL_VAL_X, ROW_Y_SET, FenceAlarmGetThresholdM(), YELLOW, BLACK);
    RangingDisplayShowAlmState(COL_VAL_X, ROW_Y_ALM, GRAY, BLACK);
}

static uint8_t RangingDisplayDrawValues(DemoResult_t *res, DemoSettings_t *cfg, uint8_t force)
{
    uint8_t showStatus;
    u16 gateColor;
    const RangingPublish_t *pub;

    if( ( force == 0u ) && ( RangingDisplayCanRefresh() == 0u ) )
    {
        return 0u;
    }

    showStatus = RangingDisplayResolveStatus(cfg->RngStatus, res);
    pub = RangingPublishGet();

    RangingDisplayShowStatusFixed(COL_VAL_X, ROW_Y_STAT, RangingDisplayStatusText(showStatus), YELLOW, BLACK);

    if( showStatus == RNG_PROCESS )
    {
        RangingDisplayShowRunPlaceholders();
        return 1u;
    }

    if( pub->round_index > 0u )
    {
        if( showStatus == RNG_DISPLAY_HOLD )
        {
            gateColor = RED;
        }
        else if( showStatus == RNG_DISPLAY_TRACK )
        {
            gateColor = YELLOW;
        }
        else if( pub->gate_reason != RNG_FENCE_GATE_OK )
        {
            gateColor = YELLOW;
        }
        else
        {
            gateColor = CYAN;
        }

        if( pub->distance_m > 0.0f )
        {
            RangingDisplayShowDistance(COL_VAL_X, ROW_Y_DIST, pub->distance_m, GREEN, BLACK);
        }
        else
        {
            RangingDisplayShowDashDist(COL_VAL_X, ROW_Y_DIST, GRAY, BLACK);
        }

        if( pub->round_distance_m > 0.0f )
        {
            RangingDisplayShowDistance(COL_VAL_X, ROW_Y_RND, pub->round_distance_m, WHITE, BLACK);
        }
        else
        {
            RangingDisplayShowDashDist(COL_VAL_X, ROW_Y_RND, GRAY, BLACK);
        }

        RangingDisplayShowU16(COL_VAL_X, ROW_Y_HOP, pub->sample_count, 2u, CYAN, BLACK);
        RangingDisplayShowGate(COL_VAL_X, ROW_Y_GATE, pub->gate_reason, gateColor, BLACK);
        RangingDisplayShowRssi(COL_VAL_X, ROW_Y_RSSI, pub->rssi_dbm, WHITE, BLACK);
        RangingDisplayShowDistance(COL_VAL_X, ROW_Y_SET, FenceAlarmGetThresholdM(), YELLOW, BLACK);
        RangingDisplayShowAlmState(COL_VAL_X, ROW_Y_ALM, WHITE, BLACK);
    }
    else
    {
        RangingDisplayShowRunPlaceholders();
    }

    (void)res;
    return 1u;
}

void RangingDisplayInit(void)
{
    DemoSettings_t *cfg = RangingDemoGetConfiguration();
    DemoResult_t *res = RangingDemoGetResultForDisplay();

    LCD_Init();
    LCD_SetInitMode( 1u );
    LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);
    LCD_ShowString(COL_LBL_X, 2, (const u8 *)"Fence", WHITE, BLACK, 12, 0);
    LCD_ShowString(COL_LBL_X, ROW_Y_STAT, (const u8 *)"Stat:", GRAY, BLACK, 12, 0);
    LCD_ShowString(COL_LBL_X, ROW_Y_DIST, (const u8 *)"Dist:", WHITE, BLACK, 12, 0);
    LCD_ShowString(COL_LBL_X, ROW_Y_RND, (const u8 *)"Rnd:", GRAY, BLACK, 12, 0);
    LCD_ShowString(COL_LBL_X, ROW_Y_HOP, (const u8 *)"Hop:", GRAY, BLACK, 12, 0);
    LCD_ShowString(COL_LBL_X, ROW_Y_GATE, (const u8 *)"Gate:", GRAY, BLACK, 12, 0);
    LCD_ShowString(COL_LBL_X, ROW_Y_RSSI, (const u8 *)"RSSI:", GRAY, BLACK, 12, 0);
    LCD_ShowString(COL_LBL_X, ROW_Y_SET, (const u8 *)"Set:", YELLOW, BLACK, 12, 0);
    LCD_ShowString(COL_LBL_X, ROW_Y_ALM, (const u8 *)"Alm:", GRAY, BLACK, 12, 0);
    LCD_SetInitMode( 0u );

    RangingDisplayDrawValues(res, cfg, 1u);
}

void RangingDisplayUpdate(void)
{
    DemoResult_t *res;
    DemoSettings_t *cfg;
    static uint16_t refresh_skip = 0u;

    RangingDemoPollRadio();

    if (RangingDisplayCanRefresh() == 0u)
    {
        return;
    }

    refresh_skip++;
    if( refresh_skip < 800u )
    {
        return;
    }
    refresh_skip = 0u;

    res = RangingDemoGetResultForDisplay();
    cfg = RangingDemoGetConfiguration();

    if( RangingDemoHasLiveSamples() )
    {
        res = RangingDemoGetResult();
    }

    RangingDisplayDrawValues(res, cfg, 0u);
}

void RangingDisplayForceUpdate(void)
{
    DemoResult_t *res;
    DemoSettings_t *cfg;

    RangingDemoPollRadio();

    res = RangingDemoGetResultForDisplay();
    cfg = RangingDemoGetConfiguration();

    if( RangingDemoHasLiveSamples() )
    {
        res = RangingDemoGetResult();
    }

    RangingDisplayDrawValues(res, cfg, 1u);
}

void RangingDisplayRefreshFenceFields(void)
{
    RangingDisplayShowDistance(COL_VAL_X, ROW_Y_SET, FenceAlarmGetThresholdM(), YELLOW, BLACK);
    RangingDisplayShowAlmState(COL_VAL_X, ROW_Y_ALM, WHITE, BLACK);
}
