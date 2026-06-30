/*
  ______                              _
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2016 Semtech

Description: Driver for SX1280 devices

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis, Matthieu Verdy and Benjamin Boulet
*/
#ifndef __RANGING_CORRECTION_H__
#define __RANGING_CORRECTION_H__

#include "rangingCorrection/rangingCorrection_defines.h"

extern const double * const RangingCorrectionPerSfBwGain[6][3];
extern const RangingCorrectionPolynomes_t * const RangingCorrectionPolynomesPerSfBw[6][3];

#endif
