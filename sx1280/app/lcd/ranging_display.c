#include "ranging_display.h"
#include "lcd.h"
#include "demoRanging.h"
#include "ranging_fence.h"
#include "ranging_publish.h"

#define RNG_DISPLAY_HOLD    250u

/* 12px 字体行距 16；RSSI 原 y=88 区域屏坏，改到 IS 之后 */
#define ROW_Y_SAMP          72u
#define ROW_Y_HTX           104u
#define ROW_Y_IS            120u
#define ROW_Y_RSSI          136u
#define COL_VAL_X           38u

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
    case RNG_INIT:
    default:
        if (status == RNG_DISPLAY_HOLD)
        {
            return (const u8 *)"Hold";
        }
        return (const u8 *)"Init";
    }
}

static void RangingDisplayShowU16(u16 x, u16 y, uint16_t num, u16 fc, u16 bc)
{
    u8 i;
    u16 div = 10000;

    for (i = 0; i < 5; i++)
    {
        u8 digit = (u8)((num / div) % 10);
        LCD_ShowChar((u16)(x + i * 6), y, (u8)('0' + digit), fc, bc, 12, 0);
        div /= 10;
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

static int8_t RangingDisplayGetRssi(DemoResult_t *res)
{
    int8_t rssi;

    if( res->RngResultIndex > 0u )
    {
        rssi = res->RawRngRssi[res->RngResultIndex - 1u];
        if( rssi != 0 )
        {
            return rssi;
        }
    }

    return (int8_t)res->RssiValue;
}

static uint16_t RangingDisplayGetHandshakeTx(DemoResult_t *res)
{
    if( ( res->CntPacketTx == 0u ) && ( res->CntPacketRxOK > 0u ) )
    {
        return 1u;
    }

    return (uint16_t)res->CntPacketTx;
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
        if( ( pub->validity == RNG_PUBLISH_OK ) || ( pub->validity == RNG_PUBLISH_RAW ) )
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

static double RangingDisplayPickDistance( DemoResult_t *res, uint8_t showStatus )
{
    const RangingPublish_t *pub = RangingPublishGet();

    if( ( pub->round_index > 0u ) && ( pub->distance_m > 0.0f ) )
    {
        return (double)pub->distance_m;
    }

    const RangingFenceOutput_t *fence = RangingFenceGetOutput();

    /* 首轮尚未 Publish 时回退 */
    if( ( showStatus == RNG_VALID ) && ( fence->gatePassed != 0u ) )
    {
        return fence->publishedDistance;
    }
    if( res->RngDistance > 0.0 )
    {
        return res->RngDistance;
    }
    if( fence->hasPublished != 0u )
    {
        return fence->publishedDistance;
    }
    return 0.0;
}

uint8_t RangingDisplayCanRefresh(void)
{
    return RangingDemoDisplayUpdateAllowed() ? 1u : 0u;
}

static void RangingDisplayShowDashField(u16 x, u16 y, u16 fc, u16 bc)
{
    LCD_ShowString(x, y, (const u8 *)"-----", fc, bc, 12, 0);
}

static void RangingDisplayShowRunPlaceholders(void)
{
    LCD_ShowString(34, 36, (const u8 *)"--.--", GRAY, BLACK, 12, 0);
    LCD_ShowString(58, 36, (const u8 *)"  ", BLACK, BLACK, 12, 0);
    RangingDisplayShowDashField(COL_VAL_X, ROW_Y_SAMP, GRAY, BLACK);
    RangingDisplayShowDashField(COL_VAL_X, ROW_Y_HTX, GRAY, BLACK);
    RangingDisplayShowDashField(COL_VAL_X, ROW_Y_IS, GRAY, BLACK);
    RangingDisplayShowRssi(COL_VAL_X, ROW_Y_RSSI, 0, GRAY, BLACK);
}

static uint8_t RangingDisplayDrawValues(DemoResult_t *res, DemoSettings_t *cfg, uint8_t force)
{
    uint8_t showStatus;
    uint16_t handshakeTx;
    uint16_t internalState;
    uint16_t sampleCount;
    int8_t rssi;
    double distance;

    if( ( force == 0u ) && ( RangingDisplayCanRefresh() == 0u ) )
    {
        return 0u;
    }

    showStatus = RangingDisplayResolveStatus(cfg->RngStatus, res);

    RangingDisplayShowStatusFixed(34, 18, RangingDisplayStatusText(showStatus), YELLOW, BLACK);

    if( showStatus == RNG_PROCESS )
    {
        RangingDisplayShowRunPlaceholders();
        return 1u;
    }

    handshakeTx = RangingDisplayGetHandshakeTx(res);
    internalState = (uint16_t)RangingDemoGetInternalState();
    sampleCount = (uint16_t)res->CntPacketRxOK;
    rssi = RangingDisplayGetRssi(res);
    distance = RangingDisplayPickDistance( res, showStatus );

    if (distance > 0.0)
    {
        RangingDisplayShowDistance(34, 36, (float)distance, GREEN, BLACK);
        LCD_ShowString(58, 36, (const u8 *)"m", CYAN, BLACK, 12, 0);
    }
    else
    {
        LCD_ShowString(34, 36, (const u8 *)"--.--", GRAY, BLACK, 12, 0);
        LCD_ShowString(58, 36, (const u8 *)"  ", BLACK, BLACK, 12, 0);
    }

    RangingDisplayShowU16(COL_VAL_X, ROW_Y_SAMP, sampleCount, CYAN, BLACK);
    RangingDisplayShowU16(COL_VAL_X, ROW_Y_HTX, handshakeTx, GRAY, BLACK);
    RangingDisplayShowU16(COL_VAL_X, ROW_Y_IS, internalState, CYAN, BLACK);
    RangingDisplayShowRssi(COL_VAL_X, ROW_Y_RSSI, rssi, WHITE, BLACK);
    return 1u;
}

void RangingDisplayInit(void)
{
    DemoSettings_t *cfg = RangingDemoGetConfiguration();
    DemoResult_t *res = RangingDemoGetResultForDisplay();

    LCD_Init();
    LCD_SetInitMode( 1u );
    LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);
    LCD_ShowString(2, 2, (const u8 *)"Ranging", WHITE, BLACK, 12, 0);
    LCD_ShowString(2, 18, (const u8 *)"Stat:", GRAY, BLACK, 12, 0);
    LCD_ShowString(2, 34, (const u8 *)"Dist:", WHITE, BLACK, 12, 0);
    LCD_ShowString(2, 72, (const u8 *)"Samp:", GRAY, BLACK, 12, 0);
    LCD_ShowString(2, 104, (const u8 *)"Htx:", GRAY, BLACK, 12, 0);
    LCD_ShowString(2, 120, (const u8 *)"IS:", GRAY, BLACK, 12, 0);
    LCD_ShowString(2, 136, (const u8 *)"RSSI:", GRAY, BLACK, 12, 0);
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
