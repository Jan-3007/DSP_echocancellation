#include "global.h"
#include "AEC.h"


AEC* g_aec;


AEC::AEC()
{
}


void AEC::init()
{
    lms_.init();

    g_codec->init();

    g_i2s_dstc->init();

    for(uint32_t i = c_delay_blocks; i > 0; i--)
    {
        delay_buffer_.fill_block(0);
    }
}


void
AEC::run()
{
    update_led();

    // error check every 1000 ms
    constexpr uint32_t max_block_count = ((1000*c_sampling_freq_Hz)/1000)/c_block_size;
    
    // start I2S and DSTC
    g_i2s_dstc->start();

    // main loop
    while(true)
    {
        if(aec_active_)
        {
            process_aec();
        }
        else
        {
            passthrough();
        }
#if 1
        if(block_counter >= max_block_count)
        {
            block_counter = 0;
            update_aec_flag();
            check_errors();
        }
#endif
    }
}


void 
AEC::create_instance()
{
    static AEC aec_instance;
    g_aec = &aec_instance;
}


void 
AEC::update_aec_flag()
{
    if(button_pressed)
    {
        if(gpio_get(USER_BUTTON) != 0)
        {
            button_pressed = false;
        }
    }
    else
    {
        if(gpio_get(USER_BUTTON) == 0)
        {
            button_pressed = true;

            if(aec_active_)
            {
                aec_active_ = false;
                update_led();
                IF_DEBUG(debug_printf("AEC off\n"));

            }
            else
            {
                aec_active_ = true;
                update_led();
                IF_DEBUG(debug_printf("AEC on\n"));
            }
        }
    }
}


void 
AEC::update_led()
{
    if(aec_active_)
    {
        // LED_R off
        gpio_set(LED_R, HIGH);
        // LED_G off
        gpio_set(LED_G, HIGH);
        // LED_B on
        gpio_set(LED_B, LOW);
    }
    else
    {
        // LED_R off
        gpio_set(LED_R, HIGH);
        // LED_B off
        gpio_set(LED_B, HIGH);
        // LED_G on
        gpio_set(LED_G, LOW);
    }
}


void 
AEC::process_aec()
{
    float32_t* r_ptr = delay_buffer_.get_write_block_ptr();
    ASSERT(r_ptr);

    // try to read input
    bool succ = g_i2s_dstc->read_rx_block_float(left_input_block_, r_ptr);
    if(!succ)
    {
        return;
    }

    gpio_set(TEST_PIN, HIGH);
    block_counter++;

    delay_buffer_.incr_write_block_ptr();

    float32_t* r_ptr_delayed = delay_buffer_.get_read_block_ptr();
    ASSERT(r_ptr_delayed);

    // process input
    lms_.process(
        r_ptr_delayed,                          // source = reference signal
        left_input_block_,                      // reference  = microphone
#if LMS_FILTER_WITH_OUTPUT
        right_output_block_,                    // output = FIR out
#endif
        left_output_block_                      // error = output
        );

    delay_buffer_.incr_read_block_ptr();

    // write output
    g_i2s_dstc->write_tx_block_float(
        left_output_block_
#if LMS_FILTER_WITH_OUTPUT
        , right_output_block_
#else
        , r_ptr_delayed
#endif
        );

    gpio_set(TEST_PIN, LOW);
}


void 
AEC::passthrough()
{
    // try to read input
    bool succ = g_i2s_dstc->read_rx_block_float(left_input_block_, right_input_block_);
    if(!succ)
    {
        return;
    }

    block_counter++;

    // output = input L
    for(uint32_t i = 0; i < c_block_size; i++)
    {
        left_output_block_[i] = left_input_block_[i];
        right_output_block_[i] = right_input_block_[i];
    }

    // write output
    g_i2s_dstc->write_tx_block_float(left_output_block_, right_output_block_);
}


void 
AEC::check_errors()
{
    I2S_DSTC::Errors errors;

    g_i2s_dstc->capture_errors(errors);

    if(errors.tx_buffer_overrun > 0)
    {
        IF_DEBUG(debug_printf("tx_buffer_overrun = %u\n", errors.tx_buffer_overrun));
    }
    if(errors.tx_buffer_underrun > 0)
    {
        IF_DEBUG(debug_printf("tx_buffer_underrun = %u\n", errors.tx_buffer_underrun));
    }
    if(errors.rx_buffer_overrun > 0)
    {
        IF_DEBUG(debug_printf("rx_buffer_overrun = %u\n", errors.rx_buffer_overrun));
    }
// rx underrun is not an error
#if 0
    if(errors.rx_buffer_underrun > 0)
    {
        IF_DEBUG(debug_printf("rx_buffer_underrun = %u\n", errors.rx_buffer_underrun));
    }
#endif
}


