#pragma once


enum aec_status
{
    SUCCESS                 =  0,        // No error 
    ERROR                   = -1,        // general error
    ARGUMENT_ERROR          = -2,        // One or more arguments are incorrect 
    LENGTH_ERROR            = -3,        // Length of data buffer is incorrect 
    BUFFER_ERROR            = -4,        // Error while writing to or reading from buffer
};



inline
void 
convert_2ch_to_float(
    const uint32_t input[c_block_size], 
    float32_t l_channel[c_block_size], 
    float32_t r_channel[c_block_size]
    )
{
    // split channels and convert to float
    for(uint32_t i = c_block_size; i > 0; i--)
    {
        uint32_t tmp = *input++;
        // right = LSB
        // left = MSB
        int16_t left = static_cast<int16_t>(tmp >> 16);
        int16_t right = static_cast<int16_t>(tmp);

        float32_t left_f = static_cast<float32_t>(left) / 32768.0f;
        float32_t right_f = static_cast<float32_t>(right) / 32768.0f;

        *l_channel++ = left_f;
        *r_channel++ = right_f;
    }
}


inline
void
convert_float_to_2ch(
    const float32_t l_channel[c_block_size], 
    const float32_t r_channel[c_block_size],
    uint32_t output[c_block_size]
    )
{
    for(uint32_t i = c_block_size; i > 0; i--)
    {
        float32_t smp_l = *l_channel++;
        float32_t smp_r = *r_channel++;

        int16_t smp_l_i = static_cast<int16_t>( __SSAT( static_cast<int32_t>(smp_l * 32768.0f), 16) );
        int16_t smp_r_i = static_cast<int16_t>( __SSAT( static_cast<int32_t>(smp_r * 32768.0f), 16) );

        uint32_t smp_l_u = static_cast<uint16_t>(smp_l_i);
        uint32_t smp_r_u = static_cast<uint16_t>(smp_r_i);

        uint32_t tmp = (smp_l_u << 16) | smp_r_u;
        *output++ = tmp;
    }
}


inline
void
convert_float_to_2ch_duplicated(
    const float32_t channel[c_block_size], 
    uint32_t output[c_block_size]
    )
{
    for(uint32_t i = c_block_size; i > 0; i--)
    {
        float32_t smp_f = *channel++;

        int16_t smp_i = static_cast<int16_t>( __SSAT( static_cast<int32_t>(smp_f * 32768.0f), 16) );

        uint32_t smp_u = static_cast<uint16_t>(smp_i);

        // duplicate signal on right and left channels
        uint32_t tmp = (smp_u << 16) | smp_u;
        *output++ = tmp;
    }
}

