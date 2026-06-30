#include "hw.h"
#include "radio.h"
#include "sx1280.h"
#include "demoRanging.h"
#include "shared_spi_bus.h"
#include "ranging_fence.h"
#include "ranging_publish.h"

/*
 * 烧录前修改 DEMO_SETTING_ENTITY 为 DEMO_ROLE_MASTER 或 DEMO_ROLE_SLAVE。
 *
 * DEMO_RNG_CONTINUOUS_MODE=1：连续短会话 + 轮末门控/滤波（app/ranging/ranging_fence.c）。
 * DEMO_RNG_CONTINUOUS_MODE=0：按键触发 demo（30 样本/轮）。
 */

#define DEMO_ROLE_MASTER  0
#define DEMO_ROLE_SLAVE   1

#define DEMO_SETTING_ENTITY  DEMO_ROLE_MASTER
//#define DEMO_SETTING_ENTITY  DEMO_ROLE_SLAVE

#define DEMO_RNG_CONTINUOUS_MODE  1

#if ( DEMO_SETTING_ENTITY == DEMO_ROLE_MASTER )
#define DEMO_USE_TFT  1
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

int main(void)
{
    RangingDemoStatus_t demoStatus;

    HwInit();

    RangingDemoInitApplication(DEMO_SETTING_ENTITY);
    RangingFenceInit();
    RangingPublishInit();

#if ( DEMO_RNG_CONTINUOUS_MODE == 1 )
    RangingDemoSetContinuousMode(1u);
    RangingDemoSetRangingParameters(RNG_CONTINUOUS_SAMPLE_COUNT, DEMO_RNG_ADDR_1, DEMO_RNG_ANT_1, DEMO_RNG_UNIT_SEL_M);
#else
    RangingDemoSetRangingParameters(30u, DEMO_RNG_ADDR_1, DEMO_RNG_ANT_1, DEMO_RNG_UNIT_SEL_M);
#endif

    RangingDemoSetRadioParameters(LORA_SF7, LORA_BW_0800, LORA_CR_4_5, DEMO_CENTRAL_FREQ_PRESET2, DEMO_POWER_TX_MAX);

#if ( DEMO_SETTING_ENTITY == DEMO_ROLE_MASTER )
    BoardUiInit();
#endif

#if ( DEMO_USE_TFT == 1 )
    RangingDisplayInit();
    SharedSpiEnsureBusIdle();
 #endif

    Radio.Reset();

#if ( DEMO_SETTING_ENTITY == DEMO_ROLE_MASTER )
    HAL_Delay(3000);
#endif

#if ( DEMO_SETTING_ENTITY == DEMO_ROLE_SLAVE )
    while( 1 )
    {
        do
        {
            demoStatus = RangingDemoRun();
        } while( demoStatus == DEMO_RANGING_RUNNING );
    }
#else /* MASTER */

#if ( DEMO_RNG_CONTINUOUS_MODE == 1 )
    while( 1 )
    {
        RangingDemoStartQuickSession();

#if ( DEMO_USE_TFT == 1 )
        RangingDisplayForceUpdate();
#endif

        do
        {
            demoStatus = RangingDemoRun();
#if ( DEMO_USE_TFT == 1 )
            RangingDemoPollRadio();
            RangingDisplayUpdate();
#endif
        } while( demoStatus == DEMO_RANGING_RUNNING );

        {
            DemoResult_t *roundResult = RangingDemoGetResult();

            RangingFenceProcessRound( roundResult, RangingDemoGetConfiguration() );  //门控
            RangingPublishUpdateFromRound( roundResult, RangingDemoGetConfiguration() ); //测距距结果发布
        }
#if ( DEMO_USE_TFT == 1 )
        RangingDisplayForceUpdate();
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

        {
            DemoResult_t *roundResult = RangingDemoGetResult();

            RangingFenceProcessRound( roundResult, RangingDemoGetConfiguration() );
            RangingPublishUpdateFromRound( roundResult, RangingDemoGetConfiguration() );
        }
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










