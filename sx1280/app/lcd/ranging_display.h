#ifndef __RANGING_DISPLAY_H
#define __RANGING_DISPLAY_H

#include <stdint.h>

void RangingDisplayInit(void);
uint8_t RangingDisplayCanRefresh(void);
void RangingDisplayUpdate(void);
void RangingDisplayForceUpdate(void);
void RangingDisplayRefreshFenceFields(void);

#endif
