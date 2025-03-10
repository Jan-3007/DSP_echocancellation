#include "global.h"





int main()
{
    // workaround for the startup script not supporting C++
    CodecWM8731::create_instance();
    I2S_DSTC::create_instance();
    AEC::create_instance();

    // GPIO port configuration for 3 color LED, user button and test pin
	GpioInit();
    gpio_set(TEST_PIN, LOW);

    IF_DEBUG(Uart0Init(115200));
    
    SystemCoreClockUpdate();
    IF_DEBUG(debug_printf("SystemCoreClock: %u Hz\n", SystemCoreClock));
    
    uint32_t msp = __get_MSP();
    IF_DEBUG(debug_printf("MSP: 0x%08x\n", msp));


    IF_DEBUG(debug_printf("AEC: %s, %s\n", __DATE__, __TIME__));
    IF_DEBUG(debug_printf("Parameters:\n"));
    IF_DEBUG(debug_printf("  Sampling frequency: %u kHz\n", (c_sampling_freq_Hz/1000)));
    IF_DEBUG(debug_printf("  Block size: %u samples\n", c_block_size));
    IF_DEBUG(debug_printf("  Ref. delay: %u blocks, %u ms\n", c_delay_blocks, ((c_delay_blocks * c_block_size * 1000) / c_sampling_freq_Hz)));
    IF_DEBUG(debug_printf("  FIR taps: %u\n", c_num_taps));
    IF_DEBUG(debug_printf("  LMS mu: %.4f\n", c_mu));


    g_aec->init();
    g_aec->run();


    
    return 0;
}

