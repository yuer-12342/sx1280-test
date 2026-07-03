#include "hw.h"
#include "radio.h"
#include "sx1280.h"
#include "demoRanging.h"
#include "shared_spi_bus.h"
#include "ranging_fence.h"
#include "ranging_publish.h"
#include "ranging_dev_log.h"
#include "fence_alarm.h"
#include "timer_hc32.h"

/*
 * 烧录前修改 DEMO_SETTING_ENTITY 为 DEMO_ROLE_MASTER 或 DEMO_ROLE_SLAVE。
 *
 * DEMO_RNG_CONTINUOUS_MODE=1：连续短会话 + 轮末门控/滤波（app/ranging/ranging_fence.c）。
 * DEMO_RNG_CONTINUOUS_MODE=0：按键触发 demo（30 样本/轮）。
 *
 * MASTER/SLAVE 共用：InitApplication、continuous 参数、SetRadioParameters（SF7+BW800）。
 * 围栏 DEMO（仅 MASTER）：短按松手切换 Set(3/5/10/15m)，长按 1s 静音；轮间 1s 也可按键。
 */

#define DEMO_ROLE_MASTER  0
#define DEMO_ROLE_SLAVE   1

#define DEMO_SETTING_ENTITY  DEMO_ROLE_MASTER
//#define DEMO_SETTING_ENTITY  DEMO_ROLE_SLAVE

#define DEMO_RNG_CONTINUOUS_MODE  1

#if ( DEMO_SETTING_ENTITY == DEMO_ROLE_MASTER )
#define DEMO_USE_TFT  1   /* 1=启用 TFT；屏坏时置 0，测距/门控/蜂鸣仍运行 */
#else
#define DEMO_USE_TFT  0
#endif

#if ( DEMO_USE_TFT == 1 )
#include "ranging_display.h"
#endif

#if ( DEMO_SETTING_ENTITY == DEMO_ROLE_MASTER )
#include "board_ui.h"
#endif

#if ( DEMO_RNG_CONTINUOUS_MODE == 0 )
static void RangingDemoKeyWaitIdle(void)
{
    (void)0;
}
#endif

#if ( DEMO_SETTING_ENTITY == DEMO_ROLE_MASTER )  //主机模式
static void RangingAppProcessRound(void)
{
    DemoResult_t *roundResult = RangingDemoGetResult();      //获取原始值

    RangingFenceProcessRound( roundResult, RangingDemoGetConfiguration() );    //门控滤波
    RangingPublishUpdateFromRound( roundResult, RangingDemoGetConfiguration() );    //状态发布
    RangingDevLogUpdate();
	  FenceAlarmOnRound();           //围栏判区+报警
}
#endif

static void RangingAppPollUi(void)
{
#if ( DEMO_SETTING_ENTITY == DEMO_ROLE_MASTER )
    FenceAlarmPollInput();         //按键检测
    FenceAlarmPollOutput();         //更新蜂鸣器状态
#if ( DEMO_USE_TFT == 1 )
    if( FenceAlarmTakeUiDirty() != 0u )
    {
        RangingDisplayRefreshFenceFields();          //刷新屏幕
    }
#endif
#endif
}

static void RangingAppBootDelayMs(uint32_t delayMs)
{
    uint32_t start = TimerGetTickMs();

    while( ( TimerGetTickMs() - start ) < delayMs )
    {
        RangingAppPollUi();
    }
}

int main(void)
{
    RangingDemoStatus_t demoStatus;

    HwInit();      //硬件初始化

    RangingDemoInitApplication(DEMO_SETTING_ENTITY);       //角色初始化

#if ( DEMO_SETTING_ENTITY == DEMO_ROLE_MASTER )
    RangingFenceInit();    //门控清零
    RangingPublishInit();    //发布清零
#endif

#if ( DEMO_RNG_CONTINUOUS_MODE == 1 )
    RangingDemoSetContinuousMode(1u);         //连续测距模式
    RangingDemoSetRangingParameters(RNG_CONTINUOUS_SAMPLE_COUNT, DEMO_RNG_ADDR_1, DEMO_RNG_ANT_1, DEMO_RNG_UNIT_SEL_M); //测距参数
#else
    RangingDemoSetRangingParameters(30u, DEMO_RNG_ADDR_1, DEMO_RNG_ANT_1, DEMO_RNG_UNIT_SEL_M);
#endif

    RangingDemoSetRadioParameters(LORA_SF7, LORA_BW_0800, LORA_CR_4_5, DEMO_CENTRAL_FREQ_PRESET2, DEMO_POWER_TX_MAX);  //射频参数

#if ( DEMO_SETTING_ENTITY == DEMO_ROLE_MASTER )
    BoardUiInit();    //按键初始化
    FenceAlarmInit();  //围栏初始化
    RangingDemoSetIdleHook( RangingAppPollUi );
#endif

#if ( DEMO_USE_TFT == 1 )
    RangingDisplayInit();
    SharedSpiEnsureBusIdle();
#endif

    Radio.Reset();

#if ( DEMO_SETTING_ENTITY == DEMO_ROLE_MASTER )
    RangingAppBootDelayMs( 3000u );
#endif

#if ( DEMO_SETTING_ENTITY == DEMO_ROLE_SLAVE )
    while( 1 )
    {
        do
        {
				demoStatus = RangingDemoRun();     //等待主机请求，响应测距
        } while( demoStatus == DEMO_RANGING_RUNNING );
    }
#else /* MASTER */

#if ( DEMO_RNG_CONTINUOUS_MODE == 1 )
    while( 1 )
    {
        RangingDemoStartQuickSession();   //发起一轮测距

#if ( DEMO_USE_TFT == 1 )
        RangingDisplayForceUpdate();
#endif

        do
        {
					demoStatus = RangingDemoRun();       //执行测距等待从机响应
#if ( DEMO_USE_TFT == 1 )
            RangingDemoPollRadio();           //轮询射频状态，处理中断
					RangingDisplayUpdate();           //更新屏幕显示
#endif
            RangingAppPollUi();              //检测按键
        } while( demoStatus == DEMO_RANGING_RUNNING );             //测距进行中则继续轮询

        RangingAppProcessRound();       //门控、发布、判区、报警
        RangingAppPollUi();             //刷新UI      

#if ( DEMO_USE_TFT == 1 )
        RangingDisplayForceUpdate();    //强制刷新
#endif
    }
#else /* button demo */
    while( 1 )
    {
        BoardUiKeyReconfigure();
        BoardUiWaitForKeyStart( RangingDemoKeyWaitIdle );

        RangingDemoReset();

        do
        {
            demoStatus = RangingDemoRun();
#if ( DEMO_USE_TFT == 1 )
            RangingDemoPollRadio();
            RangingDisplayUpdate();
#endif
        } while( demoStatus == DEMO_RANGING_RUNNING );

        RangingAppProcessRound();

#if ( DEMO_USE_TFT == 1 )
        RangingDisplayForceUpdate();
#endif

        BoardUiDisarmRanging();
        BoardUiPlayRangingCompleteAlert();
    }
#endif /* DEMO_RNG_CONTINUOUS_MODE */

#endif /* DEMO_SETTING_ENTITY */

    return 0;
}
