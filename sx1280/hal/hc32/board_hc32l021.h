#ifndef BOARD_HC32L021_H
#define BOARD_HC32L021_H

/*
 * HC32L021C8PB + SX1280 引脚（与原理图一致）
 * PA00 CS  PA01 MISO  PA02 MOSI  PA03 SCK
 * PA05 BUSY  PA08 NRST  PA15 DIO1
 *
 * 板载单天线，无外部 RF switch（Semtech 参考板 CG2214M6 天线切换不适用）
 *
 * XL1280-SMT 板载 PCB 天线测距标定（SF7 + BW800，辐射法 @ 已知距离）。
 * 11453（Semtech 表 SF7+BW800）：户外 3 m 约 7.9 m，当前基线。
 * 11000/12400 均失效（~40 m / ~51 m）；微调时从 11453 小步增加（如 11600）。
 */
#define BOARD_SINGLE_ONBOARD_ANTENNA  1
#define BOARD_RNG_CALIB_SF7_BW800     11550u

void SetAntenna1(void);
void SetAntenna2(void);

#endif
