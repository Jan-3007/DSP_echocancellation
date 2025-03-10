#include "global.h"
#include "I2S_DSTC.h"



I2S_DSTC* g_i2s_dstc;

// ctor
I2S_DSTC::I2S_DSTC()
{
}


void 
I2S_DSTC::create_instance()
{
    static I2S_DSTC i2s_dstc_instance;  
    g_i2s_dstc = &i2s_dstc_instance;
}


void 
I2S_DSTC::init()
{
    I2S0Init(c_samp_freq, dma, nullptr);

    // initial condition for tx buffer
    for(uint32_t i = c_fifo_size_blocks; i > 0; i--)
    {
        tx_buffer_.fill_block(0);
    }

    init_dstc();
}


void 
I2S_DSTC::start()
{
    // I2SEN = 1, clock to I2S macro disabled
	I2s_StartClk (&I2S0);						

	I2s_EnableTx (&I2S0);

	I2s_EnableRx (&I2S0);

	I2s_Start(&I2S0);
}


void I2S_DSTC::init_dstc()
{
    // powerup DSTC
	while ( Ok != Dstc_ReleaseStandBy() );	// wait till DSTC goes to normal state


   	PDL_ZERO_STRUCT(tx_desc_);
	PDL_ZERO_STRUCT(rx_desc_);

	// populate descriptor structures

	// channel 0 = Reads from memory (dma_tx_buffer_ping and dma_tx_buffer_pong) and transfer to the I2S peripheral
	// CH0, DES0
	tx_desc_.DES0.DV    	= 0x03;		// Don't execute the DES close process after transfer ends
	tx_desc_.DES0.ST    	= 0u;		// Just a default, DSTC updates this on completion of transfer with status information
	tx_desc_.DES0.MODE  	= 1u;      	// Mode 1 -> single transfer for 1 trigger
	tx_desc_.DES0.ORL   	= 3u;      	// Outer reload for DES1, 2
	tx_desc_.DES0.TW    	= 2u;      	// 32-bit transfer width
	tx_desc_.DES0.SAC   	= 0u;      	// source address is increased by TW * 1 at every transfer without InnerReload
	tx_desc_.DES0.DAC   	= 5u;       // Destination address remains unchanged during the transfer
	tx_desc_.DES0.CHRS  	= 0x10u;    // Interrupt flag is set when IRM = 1 and ORM = 1. No Chain start
	tx_desc_.DES0.DMSET 	= 1u;       // Set DQMSK = 1 when DES close process is executed
	tx_desc_.DES0.CHLK  	= 0u;       // No Chain start transfer
	tx_desc_.DES0.ACK   	= 1u;       // Output DMA transfer acknowledge to peripheral
	tx_desc_.DES0.RESERVED = 0u;   	// Required
	tx_desc_.DES0.PCHK  	= DSTC_PCHK_CALC(tx_desc_.u32DES0);	// parity

	// CH0, DES1, counters can be set to 1 - 256, and multiples of 256
	tx_desc_.DES1_mode1.ORM = ((c_block_size -1) >> 8) + 1;			// outer loop count
	tx_desc_.DES1_mode1.IIN = (tx_desc_.DES1_mode1.ORM > 1) ? 0 : c_block_size & 0XFF;  // Inner loop, max 256; 256 = 0
	tx_desc_.DES1_mode1.IRM = tx_desc_.DES1_mode1.IIN;			// Same as IIN

	// CH0, DES2
    uint32_t* tx_ptr = tx_buffer_.get_read_block_ptr();
    ASSERT(tx_ptr != nullptr);
	tx_desc_.DES2 = (uint32_t) tx_ptr ;   	// Source address (incremented by TW * 1 for every transfer. Configured in DES0.SAC)

	// CH0, DES3
	tx_desc_.DES3 = (uint32_t)&FM4_I2S0->TXFDAT;      	// Destination address - I2S Transmission data register (Same for every transfer,
														// configured in DES0.DAC)
	// CH0, DES4
	tx_desc_.DES4_mode1 = tx_desc_.DES1_mode1;		// Used to reload DES1

	// Ch0, DES5
	tx_desc_.DES5 = tx_desc_.DES2;					// Used to reload DES 2 in outer reload

	Dstc_SetHwdesp(DSTC_IRQ_NUMBER_I2S0_TX, 0);			// descriptor pointer start address at DESTP + offset 0 for HW channel 219




	// channel 1 = Reads from the I2S peripheral and transfer to (dma_rx_buffer_ping and dma_rx_buffer_pong)
	// CH1, DES0
	rx_desc_.DES0.DV    = 0x03;           	// Don't Execute the DES close process after transfer ends
	rx_desc_.DES0.ST    = 0u;             	// Just a default, DSTC updates this on completion of transfer with status information
	rx_desc_.DES0.MODE  = 1u;             	// Mode 1 -> single transfer for 1 trigger
	rx_desc_.DES0.ORL   = 5u;             	// Outer reload for DES1, 3
	rx_desc_.DES0.TW    = 0x2;            	// 32-bit transfer width
	rx_desc_.DES0.SAC   = 5u;             	// Source address remains unchanged during the transfer
	rx_desc_.DES0.DAC   = 0u;             	// Destination address is incremented by TW * 1 at every transfer without reload
	rx_desc_.DES0.CHRS  = 0x10u;          	// Interrupt flag is set when IRM = 1 and ORM = 1. No Chain start
	rx_desc_.DES0.DMSET = 1u;             	// Set DQMSK = 1 when DES close process is executed
	rx_desc_.DES0.CHLK  = 0u;             	// No Chain start transfer
	rx_desc_.DES0.ACK   = 1u;             	// Output DMA transfer acknowledge to peripheral
	rx_desc_.DES0.RESERVED = 0u;			// Required
	rx_desc_.DES0.PCHK  = DSTC_PCHK_CALC(rx_desc_.u32DES0);

	// CH1, DES1, counters can be set to 1 - 256, and multiples of 256
	rx_desc_.DES1_mode1.ORM = ((c_block_size -1) >> 8) + 1;		// outer loop count
	rx_desc_.DES1_mode1.IIN = (rx_desc_.DES1_mode1.ORM > 1) ? 0 : c_block_size & 0XFF;  // Inner loop, max 256; 256 = 0
	rx_desc_.DES1_mode1.IRM = rx_desc_.DES1_mode1.IIN;   		// Same as IIN

	// CH1, DES2
	rx_desc_.DES2 = (uint32_t)&FM4_I2S0->RXFDAT ;     	// Source address

	// CH1, DES3
    uint32_t* rx_ptr = rx_buffer_.get_write_block_ptr();
    ASSERT(rx_ptr != nullptr);
	rx_desc_.DES3 = (uint32_t) rx_ptr;   	// Destination address - I2S Transmission data register (Same for every transfer. Configured in DES0.DAC)

	// CH1, DES4
	rx_desc_.DES4_mode1 = rx_desc_.DES1_mode1;      	// used to reload DES1

	// CH1, DES6
	rx_desc_.DES6 = rx_desc_.DES3;					// Used to reload DES 3 in outer reload

	Dstc_SetHwdesp(DSTC_IRQ_NUMBER_I2S0_RX, 0x1C);		// descriptor pointer start address DESTP + offset 0x1C for HW channel 218
														// (7 DES0 x 4 Bytes each = 0x1C)



    // dstc config structure
	stc_dstc_config_t   	stcDstcConfig;	
  	PDL_ZERO_STRUCT(stcDstcConfig);

	// DES Top, start Address of DES Area (must be aligned to 32 Bit!)
	stcDstcConfig.u32Destp = (uint32_t) dstc_desc_;
	// TRUE: Software Interrupt enabled
	stcDstcConfig.bSwInterruptEnable = FALSE;
	// TRUE: Error Interrupt enabled
	stcDstcConfig.bErInterruptEnable = FALSE;
	// TRUE: Read Skip Buffer disabled
	stcDstcConfig.bReadSkipBufferDisable = FALSE;		// no further changes made in DES0 after init
	// TRUE: Enables Error Stop
	stcDstcConfig.bErrorStopEnable = FALSE;
	// SW transfer priority
	stcDstcConfig.enSwTransferPriority = PriorityLowest;
	// TRUE: enable NVIC
	stcDstcConfig.bTouchNvic = TRUE;
	// interrupt handler
 	stcDstcConfig.pfnDstcI2s0TxCallback = isr_tx_static;		// pointer to interrupt service routine
	stcDstcConfig.pfnDstcI2s0RxCallback = isr_rx_static;


 	Dstc_Init(&stcDstcConfig);		// write config structure to CFG register and setup interrupt callback mechanism

 	Dstc_SetCommand(CmdSwclr);		// Command to clear the SWINT interrupt
	Dstc_SetCommand(CmdErclr);		// Command to clear ERINT interrupt. MONERS.EST = 0, MONERS.DER = 0, MONERS.ESTOP = 0
	Dstc_SetCommand(CmdMkclr);		// Command to clear all DQMSK[n] registers

	Dstc_SetDreqenbBit(DSTC_IRQ_NUMBER_I2S0_TX);		// enable HW channel 219
	Dstc_SetHwintclrBit(DSTC_IRQ_NUMBER_I2S0_TX);		// Clear HWINT6 register bit corresponding to HW channel 219(I2S tx)
	Dstc_SetDreqenbBit(DSTC_IRQ_NUMBER_I2S0_RX);		// enable HW channel 218
	Dstc_SetHwintclrBit(DSTC_IRQ_NUMBER_I2S0_RX);		// Clear HWINT6 register bit corresponding to HW channel 218 (I2S rx)

}



void 
I2S_DSTC::isr_tx()
{
    // transfer is done
    tx_buffer_.incr_read_block_ptr();

    uint32_t* ptr = tx_buffer_.get_read_block_ptr();

    if(ptr == nullptr)
    {
        // error: nothing to read

        // generate one block of zeros
        tx_buffer_.fill_block(0);
        ptr = tx_buffer_.get_read_block_ptr();
    
        ASSERT(ptr != nullptr);
    }

    // new DSTC address
    tx_desc_.DES2 = reinterpret_cast<uint32_t>(ptr);

    // clear mask bit for channel 219 (I2S transmission), this reenables the DSTC
  	Dstc_SetDqmskclrBit(DSTC_IRQ_NUMBER_I2S0_TX);
}


void 
I2S_DSTC::isr_rx()
{
    // transfer is done
    rx_buffer_.incr_write_block_ptr();

    uint32_t* ptr = rx_buffer_.get_write_block_ptr();

    if(ptr == nullptr)
    {
        // error: cannot write

        // discard one block
        rx_buffer_.discard_block();
        ptr = rx_buffer_.get_write_block_ptr();

        ASSERT(ptr != nullptr);        
    }

    // new DSTC address
    rx_desc_.DES3 = reinterpret_cast<uint32_t>(ptr);

    // clear mask bit for channel 218 (I2S reception), this reenables the DSTC
	Dstc_SetDqmskclrBit(DSTC_IRQ_NUMBER_I2S0_RX);			  
}


void 
I2S_DSTC::isr_tx_static()
{
    g_i2s_dstc->isr_tx();
}


void 
I2S_DSTC::isr_rx_static()
{
    g_i2s_dstc->isr_rx();
}


bool 
I2S_DSTC::write_tx_block(const uint32_t input[])
{
    // lock out ISR
    NVIC_DisableIRQ(DSTC_HW_IRQn);

    bool succ = tx_buffer_.write_block(input);

    // unlock ISR
    NVIC_EnableIRQ(DSTC_HW_IRQn);

    return succ;
}


bool 
I2S_DSTC::read_rx_block(uint32_t output[])
{
    // lock out ISR
    NVIC_DisableIRQ(DSTC_HW_IRQn);

    if(rx_buffer_.capture_overrun_error())
    {
        fatal_error();
    }

    bool succ = rx_buffer_.read_block(output);

    // unlock ISR
    NVIC_EnableIRQ(DSTC_HW_IRQn);

    return succ;
}


bool 
I2S_DSTC::read_rx_block_float(
    float32_t l_channel[c_block_size], 
    float32_t r_channel[c_block_size]
    )
{
    // lock out ISR
    NVIC_DisableIRQ(DSTC_HW_IRQn);

    if(rx_buffer_.get_overrun_error())
    {
        fatal_error();
    }

    uint32_t* ptr = rx_buffer_.get_read_block_ptr();

    if(ptr == nullptr)
    {
        // nothing to read

        // unlock ISR
        NVIC_EnableIRQ(DSTC_HW_IRQn);

        return false;
    }
    
    // convert to float
    convert_2ch_to_float(ptr, l_channel, r_channel);

    rx_buffer_.incr_read_block_ptr();

    // unlock ISR
    NVIC_EnableIRQ(DSTC_HW_IRQn);

    return true;
}


bool 
I2S_DSTC::write_tx_block_float(
    const float32_t l_channel[c_block_size], 
    const float32_t r_channel[c_block_size]
    )
{
    // lock out ISR
    NVIC_DisableIRQ(DSTC_HW_IRQn);

    uint32_t* ptr = tx_buffer_.get_write_block_ptr();

    if(ptr == nullptr)
    {
        // unlock ISR
        NVIC_EnableIRQ(DSTC_HW_IRQn);

        return false;
    }

    convert_float_to_2ch(l_channel, r_channel, ptr);

    tx_buffer_.incr_write_block_ptr();

    // unlock ISR
    NVIC_EnableIRQ(DSTC_HW_IRQn);

    return true;
}


bool 
I2S_DSTC::write_tx_block_float(
    const float32_t channel[c_block_size]
    )
{
    // lock out ISR
    NVIC_DisableIRQ(DSTC_HW_IRQn);

    uint32_t* ptr = tx_buffer_.get_write_block_ptr();

    if(ptr == nullptr)
    {
        // unlock ISR
        NVIC_EnableIRQ(DSTC_HW_IRQn);

        return false;
    }

    convert_float_to_2ch_duplicated(channel, ptr);

    tx_buffer_.incr_write_block_ptr();

    // unlock ISR
    NVIC_EnableIRQ(DSTC_HW_IRQn);

    return true;
}


void 
I2S_DSTC::capture_errors(Errors& errors)
{
    // lock out ISR
    NVIC_DisableIRQ(DSTC_HW_IRQn);

    errors.rx_buffer_overrun = rx_buffer_.capture_overrun_error();
    errors.rx_buffer_underrun = rx_buffer_.capture_underrun_error();
    errors.tx_buffer_overrun = tx_buffer_.capture_overrun_error();
    errors.tx_buffer_underrun = tx_buffer_.capture_underrun_error();

    // unlock ISR
    NVIC_EnableIRQ(DSTC_HW_IRQn);
}


