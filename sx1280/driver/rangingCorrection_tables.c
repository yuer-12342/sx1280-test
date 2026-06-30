#include "RangingCorrection.h"

#include "rangingCorrection/rangingCorrectionSF5BW0400.h"
#include "rangingCorrection/rangingCorrectionSF6BW0400.h"
#include "rangingCorrection/rangingCorrectionSF7BW0400.h"
#include "rangingCorrection/rangingCorrectionSF8BW0400.h"
#include "rangingCorrection/rangingCorrectionSF9BW0400.h"
#include "rangingCorrection/rangingCorrectionSF10BW0400.h"
#include "rangingCorrection/rangingCorrectionSF5BW0800.h"
#include "rangingCorrection/rangingCorrectionSF6BW0800.h"
#include "rangingCorrection/rangingCorrectionSF7BW0800.h"
#include "rangingCorrection/rangingCorrectionSF8BW0800.h"
#include "rangingCorrection/rangingCorrectionSF9BW0800.h"
#include "rangingCorrection/rangingCorrectionSF10BW0800.h"
#include "rangingCorrection/rangingCorrectionSF5BW1600.h"
#include "rangingCorrection/rangingCorrectionSF6BW1600.h"
#include "rangingCorrection/rangingCorrectionSF7BW1600.h"
#include "rangingCorrection/rangingCorrectionSF8BW1600.h"
#include "rangingCorrection/rangingCorrectionSF9BW1600.h"
#include "rangingCorrection/rangingCorrectionSF10BW1600.h"

const double * const RangingCorrectionPerSfBwGain[6][3] = {
    { &RangingCorrectionSF5BW0400[0],  &RangingCorrectionSF5BW0800[0],  &RangingCorrectionSF5BW1600[0] },
    { &RangingCorrectionSF6BW0400[0],  &RangingCorrectionSF6BW0800[0],  &RangingCorrectionSF6BW1600[0] },
    { &RangingCorrectionSF7BW0400[0],  &RangingCorrectionSF7BW0800[0],  &RangingCorrectionSF7BW1600[0] },
    { &RangingCorrectionSF8BW0400[0],  &RangingCorrectionSF8BW0800[0],  &RangingCorrectionSF8BW1600[0] },
    { &RangingCorrectionSF9BW0400[0],  &RangingCorrectionSF9BW0800[0],  &RangingCorrectionSF9BW1600[0] },
    { &RangingCorrectionSF10BW0400[0], &RangingCorrectionSF10BW0800[0], &RangingCorrectionSF10BW1600[0] },
};

const RangingCorrectionPolynomes_t * const RangingCorrectionPolynomesPerSfBw[6][3] = {
    { &correctionRangingPolynomeSF5BW0400,  &correctionRangingPolynomeSF5BW0800,  &correctionRangingPolynomeSF5BW1600 },
    { &correctionRangingPolynomeSF6BW0400,  &correctionRangingPolynomeSF6BW0800,  &correctionRangingPolynomeSF6BW1600 },
    { &correctionRangingPolynomeSF7BW0400,  &correctionRangingPolynomeSF7BW0800,  &correctionRangingPolynomeSF7BW1600 },
    { &correctionRangingPolynomeSF8BW0400,  &correctionRangingPolynomeSF8BW0800,  &correctionRangingPolynomeSF8BW1600 },
    { &correctionRangingPolynomeSF9BW0400,  &correctionRangingPolynomeSF9BW0800,  &correctionRangingPolynomeSF9BW1600 },
    { &correctionRangingPolynomeSF10BW0400, &correctionRangingPolynomeSF10BW0800, &correctionRangingPolynomeSF10BW1600 },
};
