#include "board_ui.h"
#include "board_stkhc32l021.h"
#include "atim3.h"
#include "gpio.h"
#include "sysctrl.h"
#include "ddl.h"
#include "hc32l021.h"

/* PA09: ATIM3_CH1B，无源蜂鸣器 PWM（参考 atimer3_m23_saw_pwm） */
#define BUZZER_PWM_ARR    2222u
#define BUZZER_PWM_CMP    1111u

#define UI_BEEP_ON_MS     80u
#define UI_BEEP_OFF_MS    80u
#define UI_CYCLE_GAP_MS   300u
#define UI_ALERT_CYCLES   3u
#define UI_BEEPS_PER_CYCLE 3u

#define UI_KEY_DEBOUNCE_MS  30u

volatile uint8_t g_BoardUiKeyRaw       = 1u;
volatile uint8_t g_BoardUiKeyPressed   = 0u;
volatile uint8_t g_BoardUiRangingArmed = 0u;
volatile uint8_t g_BoardUiWaitPhase    = UI_KEY_PHASE_IDLE;
volatile uint32_t g_BoardUiGpioAIn     = 0u;

static uint8_t s_rangingArmed = 0u;

static void BoardUiKeyConfigure(void)
{
    stc_gpio_init_t stcGpioInit = {0};

    SYSCTRL_PeriphClockEnable(PeriphClockGpio);
    GPIO_PA12_AF_GPIO_SET();
    GPIO_StcInit(&stcGpioInit);
    stcGpioInit.u32Pin    = GPIO_PIN_12;
    stcGpioInit.u32Mode   = GPIO_MD_INPUT;
    stcGpioInit.u32PullUp = GPIO_PULL_UP;
    GPIOA_Init(&stcGpioInit);

    /* 确保 PA12 为 GPIO 输入 + 上拉（与官方 gpio_input 改上拉后一致） */
    CLR_REG_BIT(GPIOA->ADS, GPIO_PIN_12);
    SET_REG_BIT(GPIOA->DIR, GPIO_PIN_12);
    SET_REG_BIT(GPIOA->PU, GPIO_PIN_12);
}

static void BoardUiBuzzerPwmInit(void)
{
    uint16_t                     u16CntValue;
    stc_atim3_mode23_init_t      stcAtim3BaseCfg    = {0};
    stc_atim3_m23_compare_init_t stcAtim3ChxBCmpCfg = {0};
    stc_gpio_init_t              stcGpioInit        = {0};

    SYSCTRL_PeriphClockEnable(PeriphClockGpio);
    SYSCTRL_PeriphClockEnable(PeriphClockAtim3);

    GPIO_StcInit(&stcGpioInit);
    stcGpioInit.u32Pin    = GPIO_PIN_09;
    stcGpioInit.u32Mode   = GPIO_MD_OUTPUT_PP;
    stcGpioInit.u32PullUp = GPIO_PULL_NONE;
    GPIOA_Init(&stcGpioInit);
    GPIO_PA09_AF_ATIM3_CH1B();

    ATIM3_Mode23_StcInit(&stcAtim3BaseCfg);
    stcAtim3BaseCfg.u32WorkMode         = ATIM3_M23_M23CR_WORK_MODE_SAWTOOTH;
    stcAtim3BaseCfg.u32CountClockSelect = ATIM3_M23_M23CR_CT_PCLK;
    stcAtim3BaseCfg.u32PRS              = ATIM3_M23_M23CR_ATIM3CLK_PRS1;
    stcAtim3BaseCfg.u32CountDir         = ATIM3_M23_M23CR_DIR_UP_CNT;
    stcAtim3BaseCfg.u32PWMTypeSelect    = ATIM3_M23_M23CR_COMP_PWM_INDEPT;
    stcAtim3BaseCfg.u32PWM2sSelect      = ATIM3_M23_M23CR_PWM2S_COMPARE_SINGLE_POINT;
    stcAtim3BaseCfg.u32ShotMode         = ATIM3_M23_M23CR_SHOT_CYCLE;
    stcAtim3BaseCfg.u32URSSelect        = ATIM3_M23_M23CR_URS_OV_UND;
    ATIM3_Mode23_Init(&stcAtim3BaseCfg);

    ATIM3_Mode23_ARRSet(BUZZER_PWM_ARR);
    ATIM3_Mode23_ARRBufferEnable();
    ATIM3_Mode23_ChannelCompareValueSet(ATIM3_COMPARE_CAPTURE_CH1B, BUZZER_PWM_CMP);

    ATIM3_Mode23_CompareStcInit(&stcAtim3ChxBCmpCfg);
    stcAtim3ChxBCmpCfg.u32CHxCompareCap       = ATIM3_M23_CRCHx_CSA_CSB_COMPARE;
    stcAtim3ChxBCmpCfg.u32CHxCompareModeCtrl = ATIM3_M23_FLTR_OCMxx_PWM_MODE_2;
    stcAtim3ChxBCmpCfg.u32CHxPolarity         = ATIM3_M23_FLTR_CCPxx_NORMAL_IN_OUT;
    stcAtim3ChxBCmpCfg.u32CHxCompareBufEn     = ATIM3_M23_CRCHx_BUFEx_ENABLE;
    stcAtim3ChxBCmpCfg.u32CHxCompareIntSelect = ATIM3_M23_CRCHx_CISB_DISABLE_INT;
    ATIM3_Mode23_PortOutputCHxBConfig(ATIM3_M23_OUTPUT_CHANNEL_CH1, &stcAtim3ChxBCmpCfg);

    u16CntValue = 0u;
    ATIM3_Mode23_Cnt16Set(u16CntValue);
    ATIM3_IntFlagClearAll();
}

static void BoardUiBuzzerOn(void)
{
    ATIM3_Mode23_Run();
    ATIM3_Mode23_ManualPWMOutputEnable();
}

static void BoardUiBuzzerOff(void)
{
    ATIM3_Mode23_ManualPWMOutputDisable();
    ATIM3_Mode23_Stop();
}

void BoardUiInit(void)
{
    STK_LedConfig();
    BoardUiKeyConfigure();
    STK_LED_OFF();
    BoardUiBuzzerPwmInit();
    BoardUiBuzzerOff();
    BoardUiDisarmRanging();
    g_BoardUiWaitPhase = UI_KEY_PHASE_IDLE;
}

void BoardUiKeyReconfigure(void)
{
    BoardUiKeyConfigure();
    BoardUiKeyPressed();
}

uint8_t BoardUiKeyPressed(void)
{
    g_BoardUiGpioAIn = GPIOA->IN;
    /* READ_REG_BIT 返回 0 或 GPIO_PIN_12(0x1000)，不可直接 cast 到 uint8_t */
    g_BoardUiKeyRaw = ( GPIO_PA12_READ() != 0u ) ? 1u : 0u;
    g_BoardUiKeyPressed = ( g_BoardUiKeyRaw == 0u ) ? 1u : 0u;
    return g_BoardUiKeyPressed;
}

void BoardUiDebugRefreshKey(void)
{
    (void)BoardUiKeyPressed();
}

void BoardUiWaitKeyReleasedAtBoot(void)
{
    uint16_t stableMs = 0u;

    while( stableMs < UI_KEY_DEBOUNCE_MS )
    {
        if( BoardUiKeyPressed() == 0u )
        {
            stableMs++;
        }
        else
        {
            stableMs = 0u;
        }
        DDL_Delay1ms(1);
    }
}

void BoardUiArmRanging(void)
{
    s_rangingArmed = 1u;
    g_BoardUiRangingArmed = 1u;
}

void BoardUiDisarmRanging(void)
{
    s_rangingArmed = 0u;
    g_BoardUiRangingArmed = 0u;
}

uint8_t BoardUiIsRangingArmed(void)
{
    return s_rangingArmed;
}

static uint8_t BoardUiWaitKeyStable(uint8_t pressedLevel, BoardUiIdleHook idleHook)
{
    uint16_t stableMs = 0u;

    while( stableMs < UI_KEY_DEBOUNCE_MS )
    {
        if( BoardUiKeyPressed() == pressedLevel )
        {
            stableMs++;
        }
        else
        {
            stableMs = 0u;
        }

        if( idleHook != NULL )
        {
            idleHook();
        }

        DDL_Delay1ms(1);
    }

    return 1u;
}

void BoardUiWaitForKeyStart(BoardUiIdleHook idleHook)
{
    BoardUiDisarmRanging();
    BoardUiKeyReconfigure();

    /* phase 1: 等稳定按下（连续 30ms 读为按下） */
    g_BoardUiWaitPhase = UI_KEY_PHASE_PRESS;
    (void)BoardUiWaitKeyStable(1u, idleHook);

    /* phase 2: 等稳定松开（连续 30ms 读为松开） */
    g_BoardUiWaitPhase = UI_KEY_PHASE_RELEASE;
    (void)BoardUiWaitKeyStable(0u, idleHook);

    g_BoardUiWaitPhase = UI_KEY_PHASE_DONE;
    STK_LED_ON();
    DDL_Delay1ms(100);
    STK_LED_OFF();

    BoardUiArmRanging();
}

void BoardUiPlayRangingCompleteAlert(void)
{
    uint8_t cycle;
    uint8_t beep;

    g_BoardUiWaitPhase = UI_KEY_PHASE_IDLE;

    for( cycle = 0u; cycle < UI_ALERT_CYCLES; cycle++ )
    {
        for( beep = 0u; beep < UI_BEEPS_PER_CYCLE; beep++ )
        {
            BoardUiBuzzerOn();
            STK_LED_ON();
            DDL_Delay1ms(UI_BEEP_ON_MS);
            BoardUiBuzzerOff();
            STK_LED_OFF();
            DDL_Delay1ms(UI_BEEP_OFF_MS);
        }

        if( cycle < ( UI_ALERT_CYCLES - 1u ) )
        {
            DDL_Delay1ms(UI_CYCLE_GAP_MS);
        }
    }
}

void BoardUiBuzzerSet(uint8_t on)
{
    if( on != 0u )
    {
        BoardUiBuzzerOn();
    }
    else
    {
        BoardUiBuzzerOff();
    }
}

void BoardUiLedSet(uint8_t on)
{
    if( on != 0u )
    {
        STK_LED_ON();
    }
    else
    {
        STK_LED_OFF();
    }
}

void BoardUiPlayKeyClick(void)
{
    BoardUiBuzzerOn();
    DDL_Delay1ms( 40u );
    BoardUiBuzzerOff();
}

void BoardUiPlayKeyLongPress(void)
{
    BoardUiBuzzerOn();
    DDL_Delay1ms( 120u );
    BoardUiBuzzerOff();
}
