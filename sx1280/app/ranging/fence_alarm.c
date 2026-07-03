/*
 * 室内粗测：Set 默认 0.6 m，用 Rnd 判距，Hold 也计数，连续 2 轮触发。
 * 仅改此处并 Rebuild 即可切换。
 */
#ifndef FENCE_ALARM_INDOOR_DEMO
#define FENCE_ALARM_INDOOR_DEMO  0
#endif

#include "fence_alarm.h"
#include "ranging_publish.h"
#include "board_ui.h"
#include "timer_hc32.h"

#define FENCE_ALARM_PRESET_COUNT        4u
/* 短按：按下 ≥30 ms 且在 3 s 前松开 → 切换 Set（短促嘀一声）
 * 长按：按住满 3 s → 开关报警（较长嘀一声），松开不再触发短按 */
#define FENCE_ALARM_KEY_LONG_MS         3000u
#define FENCE_ALARM_KEY_MIN_MS          30u
#define FENCE_ALARM_BEEP_ON_MS          150u
#define FENCE_ALARM_BEEP_OFF_MS         850u

#if ( FENCE_ALARM_INDOOR_DEMO != 0 )
#define FENCE_ALARM_HYSTERESIS_M        0.3f
#define FENCE_ALARM_DEFAULT_PRESET_IDX  0u
#define FENCE_ALARM_TRIGGER_ROUNDS      2u
#define FENCE_ALARM_CLEAR_ROUNDS        2u
static const float s_presets[FENCE_ALARM_PRESET_COUNT] = { 0.6f, 0.8f, 1.0f, 1.5f };
#else
#define FENCE_ALARM_HYSTERESIS_M        1.0f
#define FENCE_ALARM_DEFAULT_PRESET_IDX  1u
#define FENCE_ALARM_TRIGGER_ROUNDS      3u
#define FENCE_ALARM_CLEAR_ROUNDS        3u
static const float s_presets[FENCE_ALARM_PRESET_COUNT] = { 3.0f, 5.0f, 10.0f, 15.0f };
#endif

static uint8_t s_presetIndex;
static uint8_t s_enabled;
static uint8_t s_overCount;
static uint8_t s_underCount;
static FenceAlarmState_t s_state;

static uint8_t s_keyDown;
static uint8_t s_keyLongDone;
static uint32_t s_keyDownMs;
static uint8_t s_uiDirty;

static uint8_t s_buzzerOn;
static uint32_t s_beepPhaseMs;

static float FenceAlarmGetClearThresholdM( void )
{
    float clearM = FenceAlarmGetThresholdM() - FENCE_ALARM_HYSTERESIS_M;

#if ( FENCE_ALARM_INDOOR_DEMO != 0 )
    if( clearM < 0.2f )
    {
        clearM = 0.2f;
    }
#else
    if( clearM < 0.5f )
    {
        clearM = 0.5f;
    }
#endif

    return clearM;
}

#if ( FENCE_ALARM_INDOOR_DEMO != 0 )
static uint8_t FenceAlarmPickDistanceM( const RangingPublish_t *pub, float *distanceM )
{
    if( pub->validity == RNG_PUBLISH_INVALID )
    {
        return 0u;
    }

    if( pub->round_distance_m > 0.0f )
    {
        *distanceM = pub->round_distance_m;
        return 1u;
    }

    if( pub->distance_m > 0.0f )
    {
        *distanceM = pub->distance_m;
        return 1u;
    }

    return 0u;
}
#else
static uint8_t FenceAlarmPickDistanceM( const RangingPublish_t *pub, float *distanceM )
{
    if( pub->validity != RNG_PUBLISH_OK )
    {
        return 0u;
    }

    *distanceM = pub->distance_m;
    return 1u;
}
#endif

void FenceAlarmInit( void )
{
    s_presetIndex  = FENCE_ALARM_DEFAULT_PRESET_IDX;
    s_enabled      = 1u;
    s_overCount    = 0u;
    s_underCount   = 0u;
    s_state        = FENCE_ALARM_STATE_SAFE;
    s_keyDown      = 0u;
    s_keyLongDone  = 0u;
    s_keyDownMs    = 0u;
    s_uiDirty      = 0u;
    s_buzzerOn     = 0u;
    s_beepPhaseMs  = 0u;
    BoardUiBuzzerSet( 0u );
    BoardUiLedSet( 0u );
}

float FenceAlarmGetThresholdM( void )
{
    return s_presets[s_presetIndex];
}

uint8_t FenceAlarmGetPresetIndex( void )
{
    return s_presetIndex;
}

uint8_t FenceAlarmGetPresetCount( void )
{
    return FENCE_ALARM_PRESET_COUNT;
}

FenceAlarmState_t FenceAlarmGetState( void )
{
    return s_state;
}

uint8_t FenceAlarmIsEnabled( void )
{
    return s_enabled;
}

uint8_t FenceAlarmTakeUiDirty( void )
{
    uint8_t dirty = s_uiDirty;

    s_uiDirty = 0u;
    return dirty;
}

static void FenceAlarmCyclePreset( void )
{
    s_presetIndex = (uint8_t)( ( s_presetIndex + 1u ) % FENCE_ALARM_PRESET_COUNT );
    s_overCount   = 0u;
    s_underCount  = 0u;
    s_uiDirty     = 1u;
    BoardUiPlayKeyClick( );
}

static void FenceAlarmToggleEnabled( void )
{
    s_enabled = (uint8_t)( s_enabled == 0u );
    if( s_enabled == 0u )
    {
        s_state      = FENCE_ALARM_STATE_SAFE;
        s_overCount  = 0u;
        s_underCount = 0u;
        BoardUiBuzzerSet( 0u );
        BoardUiLedSet( 0u );
    }
    s_uiDirty = 1u;
    BoardUiPlayKeyLongPress( );
}

void FenceAlarmPollInput( void )
{
    uint8_t pressed;
    uint32_t now;
    uint32_t heldMs;

    pressed = BoardUiKeyPressed();
    now     = TimerGetTickMs();

    if( pressed != 0u )
    {
        if( s_keyDown == 0u )
        {
            s_keyDownMs   = now;
            s_keyDown     = 1u;
            s_keyLongDone = 0u;
        }
        else if( ( s_keyLongDone == 0u ) &&
                 ( ( now - s_keyDownMs ) >= FENCE_ALARM_KEY_LONG_MS ) )
        {
            FenceAlarmToggleEnabled();
            s_keyLongDone = 1u;
        }
        return;
    }

    if( s_keyDown != 0u )
    {
        heldMs = now - s_keyDownMs;
        if( ( s_keyLongDone == 0u ) &&
            ( heldMs >= FENCE_ALARM_KEY_MIN_MS ) &&
            ( heldMs < FENCE_ALARM_KEY_LONG_MS ) )
        {
            FenceAlarmCyclePreset();
        }
        s_keyDown     = 0u;
        s_keyLongDone = 0u;
    }
}

void FenceAlarmOnRound( void )
{
    const RangingPublish_t *pub = RangingPublishGet();
    float thresholdM;
    float clearM;
    float distanceM;

    if( s_enabled == 0u )
    {
        return;
    }

    if( FenceAlarmPickDistanceM( pub, &distanceM ) == 0u )
    {
        return;
    }

    thresholdM = FenceAlarmGetThresholdM();
    clearM       = FenceAlarmGetClearThresholdM();

    if( distanceM > thresholdM )
    {
        s_overCount++;
        s_underCount = 0u;
        if( s_overCount >= FENCE_ALARM_TRIGGER_ROUNDS )
        {
            if( s_state != FENCE_ALARM_STATE_OUT )
            {
                s_beepPhaseMs = TimerGetTickMs();
            }
            s_state = FENCE_ALARM_STATE_OUT;
        }
    }
    else if( distanceM < clearM )
    {
        s_underCount++;
        s_overCount = 0u;
        if( s_underCount >= FENCE_ALARM_CLEAR_ROUNDS )
        {
            s_state = FENCE_ALARM_STATE_SAFE;
        }
    }
}

void FenceAlarmPollOutput( void )
{
    uint32_t now;
    uint32_t phase;

    if( ( s_enabled == 0u ) || ( s_state != FENCE_ALARM_STATE_OUT ) )
    {
        if( s_buzzerOn != 0u )
        {
            BoardUiBuzzerSet( 0u );
            s_buzzerOn = 0u;
        }
        BoardUiLedSet( 0u );
        return;
    }

    BoardUiLedSet( 1u );

    now   = TimerGetTickMs();
    phase = ( now - s_beepPhaseMs ) % ( FENCE_ALARM_BEEP_ON_MS + FENCE_ALARM_BEEP_OFF_MS );

    if( phase < FENCE_ALARM_BEEP_ON_MS )
    {
        if( s_buzzerOn == 0u )
        {
            BoardUiBuzzerSet( 1u );
            s_buzzerOn = 1u;
        }
    }
    else
    {
        if( s_buzzerOn != 0u )
        {
            BoardUiBuzzerSet( 0u );
            s_buzzerOn = 0u;
        }
    }
}
