/* platform.h
 * Platform initialization, enables access to platform resources like I2S0, audio CODEC, UART0 and gpio
 *
 *  Copyright (C) 2019 Thomas Erforth
 *
 * Change history:
 * ter, Jan 30, 2019: Initial version
 * ter, Apr 09, 2019: enum mode: poll added
 * ter, July 21, 2019: Declarations for dma_buffers, DSTC and slider app moved to platform.c and declared extern here
 * 					   Define directive for PING / PONG added
 *
 */

#ifndef platform_H
#define platform_H

#include <pdl_header.h>
#include <FM4_slider_interface.h>


// channel assignment for union audio
#define LEFT 1
#define RIGHT 0

// GPIO
#define HIGH 1
#define LOW 0
#define ENABLE 1
#define DISABLE 0

// DMA with DSTC
#define DMA_BUFFER_SIZE 512 // 1 - 256 and multiples of 256 (e.g. 512, 768, 1024, ...)
#define PING 0x00
#define PONG 0x01

// initial settings for slider app
#define BAUDRATE 460800		// UART0 baud rate used for slider app, choose an appropriate value,
							// typical values for a pc system are 921600, 460800, 230400, 115200 etc.
#define SLIDER4	24.0		// used for line in gain in function setLIandHPlevels; 23.0: 0 dB, max 31.0: 12 dB, min 0.0: -31.5 db; 1.5 dB steps
#define SLIDER5	126.0		// used for HP out gain in function setLIandHPlevels; 47.0: mute, 121.0: 0 dB, max 127.0: 6 dB, min 48.0: -73 dB; 1 dB steps
#define SLIDER0 0.0
#define SLIDER1 0.0
#define SLIDER2 0.0
#define SLIDER3 0.0

// Variables used for DSTC buffer handling
extern uint32_t dma_tx_buffer_pong[DMA_BUFFER_SIZE], dma_rx_buffer_pong[DMA_BUFFER_SIZE],
				dma_tx_buffer_ping[DMA_BUFFER_SIZE], dma_rx_buffer_ping[DMA_BUFFER_SIZE];

extern volatile uint8_t rx_proc_buffer;
extern volatile uint8_t tx_proc_buffer;
extern volatile boolean_t tx_buffer_empty;
extern volatile boolean_t rx_buffer_full;

extern stc_dstc_des0123456_t stcDES[2];     // Instance for DSTC Descriptors 0 - 4

// Create (instantiate) GUI slider data structure
extern struct FM4_slider_struct FM4_GUI;

// union used to move the I2S data
union audio {
	int32_t audioSample;
	int16_t audio_ch[2];
};


/*! This enum describes the different sampling frequency setup of the CODEC */
typedef enum
{
	hz48000 = 0x00,  // 48kHz from 12.288MHz MCLK
    hz8000 	= 0x0C,  // 8kHz from 12.288MHz MCLK
    hz32000 = 0x18,  // 32kHz from 12.288MHz MCLK
    hz96000 = 0x1C,  // 96kHz from 12.288MHz MCLK
} sampling_rate;

/*! This enum describes the different input setup of the CODEC */		
typedef enum
{
	line_in = 0x12,  // Select DAC input Line In and disable bypass
    mic_in 	= 0x15,  // Select DAC input Microphone In and disable bypass
} audio_input;

/*! This enum describes the different operating modes of the I2S interface */
typedef enum
{
	intr 	= 0x00,     // User I2S interruptions to control the flow of the program (generate interruption when  irq_depth=<FIFO level)
    dma		= 0x01,     // Use DMA requests to control the flow of the program (generate a request when dma_depth=<FIFO level)
	poll	= 0x02,		// polling, no interrupts used
} mode;


/* Prototypes */

void platform_init (uint32_t, uint32_t, audio_input, mode, void (*)(void), void (*)(void));
void Uart0Init(uint32_t);
void CodecInit(sampling_rate, audio_input);
void GpioInit(void);
void I2S0Init(sampling_rate, mode, void (*)(void));
void DstcInit(void (*)(void), void (*)(void));
void DSTC_I2S_HANDLER_CH0 (void);
void DSTC_I2S_HANDLER_CH1 (void);

	
#endif // platform_H

