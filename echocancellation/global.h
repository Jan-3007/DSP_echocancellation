#pragma once


// C++
#include <stdio.h>
#include <math.h>


// PDL
extern "C"
{
#include <platform.h>
#include <utils.h>
}


// CMSIS-DSP
#include "arm_math.h"


// custom
#include "debug_utils.h"
#include "aec_config.h"
#include "aec_utils.h"
#include "CircularBuffer.h"
#include "LMSFilter.h"
#include "CodecWM8731.h"
#include "I2S_DSTC.h"
#include "AEC.h"






