#ifndef BOARD_UI_H
#define BOARD_UI_H

#include <stdint.h>

typedef void (*BoardUiIdleHook)(void);

#define UI_KEY_PHASE_IDLE     0u
#define UI_KEY_PHASE_PRESS    1u
#define UI_KEY_PHASE_RELEASE  2u
#define UI_KEY_PHASE_DONE     3u

extern volatile uint8_t g_BoardUiKeyRaw;       /* PA12 电平：1=松开，0=按下（与 GPIO 读一致） */
extern volatile uint8_t g_BoardUiKeyPressed;   /* 1=按下 */
extern volatile uint8_t g_BoardUiRangingArmed;
extern volatile uint8_t g_BoardUiWaitPhase;
extern volatile uint32_t g_BoardUiGpioAIn;     /* GPIOA->IN 整字，PA12=bit12(0x1000) */

void BoardUiDebugRefreshKey(void);             /* 调试暂停时可手动刷新按键 Watch */

void BoardUiInit(void);
void BoardUiKeyReconfigure(void);
uint8_t BoardUiKeyPressed(void);
void BoardUiWaitKeyReleasedAtBoot(void);
void BoardUiWaitForKeyStart(BoardUiIdleHook idleHook);
void BoardUiArmRanging(void);
void BoardUiDisarmRanging(void);
uint8_t BoardUiIsRangingArmed(void);
void BoardUiPlayRangingCompleteAlert(void);
void BoardUiBuzzerSet(uint8_t on);
void BoardUiLedSet(uint8_t on);
void BoardUiPlayKeyClick(void);
void BoardUiPlayKeyLongPress(void);

#endif
