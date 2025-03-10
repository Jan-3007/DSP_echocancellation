#include "global.h"


// ctor
LMSFilter::LMSFilter()
{
}


void
LMSFilter::init()
{
    // clear the state buffer
    memset(data_, 0, sizeof(data_));

    // clear the coeffs buffer
    memset(coeffs_, 0, sizeof(coeffs_));
}


void
LMSFilter::process(
    const float32_t* source,        // points to the block of input data
    const float32_t* reference,     // points to the block of reference data
#if LMS_FILTER_WITH_OUTPUT
    float32_t* output,              // points to the block of output data
#endif
    float32_t* error                // points to the block of error data        
    )
{
    float32_t* data_ptr = data_;
    float32_t* new_data = &data_[c_num_taps - 1];

    // process data and append samples from source to data    
    for(uint32_t sample_cnt = c_block_size; sample_cnt > 0; sample_cnt--)
    {
        // copy new sample into the data buffer, included here to avoid another loop
        *new_data++ = *source++;

        // FIR-filter: apply coefficients to data
        float32_t* current_data = data_ptr;
        float32_t* coeff_ptr = coeffs_;
        float32_t acc = 0.0f;

#if LMS_FILTER_LOOP_UNROLLING
        static_assert((c_num_taps % 16) == 0, "c_num_taps must be multiples of 16");

        for(uint32_t tap_cnt = (c_num_taps / 16); tap_cnt > 0; tap_cnt--)
        {
            // multiply-accumulate
            acc += (*current_data++) * (*coeff_ptr++);
            acc += (*current_data++) * (*coeff_ptr++);
            acc += (*current_data++) * (*coeff_ptr++);
            acc += (*current_data++) * (*coeff_ptr++);

            acc += (*current_data++) * (*coeff_ptr++);
            acc += (*current_data++) * (*coeff_ptr++);
            acc += (*current_data++) * (*coeff_ptr++);
            acc += (*current_data++) * (*coeff_ptr++);

            acc += (*current_data++) * (*coeff_ptr++);
            acc += (*current_data++) * (*coeff_ptr++);
            acc += (*current_data++) * (*coeff_ptr++);
            acc += (*current_data++) * (*coeff_ptr++);

            acc += (*current_data++) * (*coeff_ptr++);
            acc += (*current_data++) * (*coeff_ptr++);
            acc += (*current_data++) * (*coeff_ptr++);
            acc += (*current_data++) * (*coeff_ptr++);
        }
#else
        for(uint32_t tap_cnt = c_num_taps; tap_cnt > 0; tap_cnt--)
        {
            // multiply-accumulate
            acc += (*current_data++) * (*coeff_ptr++);
        }
#endif

#if LMS_FILTER_WITH_OUTPUT
        // store result in output
        *output++ = acc;
#endif

        // compute error
        float32_t e = *reference++ - acc;
        *error++ = e;

        // compute weight for updating filter coeffs
        float32_t weight = e * c_mu;

        // reset data and coefficients pointer
        current_data = data_ptr++;
        coeff_ptr = coeffs_;

        // apply weight to coefficients
#if LMS_FILTER_LOOP_UNROLLING
        for(uint32_t tap_cnt = (c_num_taps / 16); tap_cnt > 0; tap_cnt--)
        {
            (*coeff_ptr++) += (weight * (*current_data++));          
            (*coeff_ptr++) += (weight * (*current_data++));          
            (*coeff_ptr++) += (weight * (*current_data++));          
            (*coeff_ptr++) += (weight * (*current_data++));          

            (*coeff_ptr++) += (weight * (*current_data++));          
            (*coeff_ptr++) += (weight * (*current_data++));          
            (*coeff_ptr++) += (weight * (*current_data++));          
            (*coeff_ptr++) += (weight * (*current_data++));          

            (*coeff_ptr++) += (weight * (*current_data++));          
            (*coeff_ptr++) += (weight * (*current_data++));          
            (*coeff_ptr++) += (weight * (*current_data++));          
            (*coeff_ptr++) += (weight * (*current_data++));          

            (*coeff_ptr++) += (weight * (*current_data++));          
            (*coeff_ptr++) += (weight * (*current_data++));          
            (*coeff_ptr++) += (weight * (*current_data++));          
            (*coeff_ptr++) += (weight * (*current_data++));          
        }
#else
        for(uint32_t tap_cnt = c_num_taps; tap_cnt > 0; tap_cnt--)
        {
            (*coeff_ptr++) += (weight * (*current_data++));          
        }
#endif
    }

    // processing is complete
    // prepare state buffer for the next process call

    // copy appended data to front of data
    float32_t* current_data = data_;
#if LMS_FILTER_LOOP_UNROLLING
    for(uint32_t tap_cnt = (c_num_taps / 16); tap_cnt > 0; tap_cnt--)
    {
        *current_data++ = *data_ptr++;
        *current_data++ = *data_ptr++;
        *current_data++ = *data_ptr++;
        *current_data++ = *data_ptr++;

        *current_data++ = *data_ptr++;
        *current_data++ = *data_ptr++;
        *current_data++ = *data_ptr++;
        *current_data++ = *data_ptr++;

        *current_data++ = *data_ptr++;
        *current_data++ = *data_ptr++;
        *current_data++ = *data_ptr++;
        *current_data++ = *data_ptr++;

        *current_data++ = *data_ptr++;
        *current_data++ = *data_ptr++;
        *current_data++ = *data_ptr++;
        *current_data++ = *data_ptr++;
    }
#else
    for(uint32_t tap_cnt = c_num_taps - 1; tap_cnt > 0; tap_cnt--)
    {
        *current_data++ = *data_ptr++;
    }
#endif
}





