#pragma once


#ifndef LMS_FILTER_LOOP_UNROLLING
#define LMS_FILTER_LOOP_UNROLLING 1
#endif

#ifndef LMS_FILTER_WITH_OUTPUT
#define LMS_FILTER_WITH_OUTPUT 0
#endif


class LMSFilter
{
protected:
    // coefficients are stored in time reversed order
    float32_t coeffs_[ c_num_taps ];
    float32_t data_[ (c_num_taps - 1) + c_block_size ];


public:
    // ctor
    LMSFilter();

    void
    init();

    void
    process(
        const float32_t* source,        // points to the block of input data
        const float32_t* reference,     // points to the block of reference data
#if LMS_FILTER_WITH_OUTPUT
        float32_t* output,              // points to the block of output data
#endif
        float32_t* error                // points to the block of error data
        );

};

