#pragma once



//------------------------------
//
// General
//
//------------------------------

constexpr uint32_t c_sampling_freq_Hz = 32000;

// number of samples per block
// must be power of 2
// @48kHz   -> 48 samples/block = 1 ms sampling time
constexpr uint32_t c_block_size = 64;

// number of blocks to delay reference signal
// this compensates the significant delay caused by the USB microphone
// current hardware setup requires a delay of about 93 ms
constexpr uint32_t c_delay_blocks = ((93*c_sampling_freq_Hz)/1000)/c_block_size;



//------------------------------
//
// Codec WM8731
//
//------------------------------

// sampling frequency
// 96 kHz, 48 kHz, 32 kHz, 8 kHz
constexpr sampling_rate c_samp_freq = hz32000;

// DAC input: line_in, mic_in
// line_in -> 2 channels
// mic_in -> 1 channel
constexpr audio_input c_audio_source = line_in;



//------------------------------
//
// RX/TX DMA FIFO
//
//------------------------------

// number of blocks the buffer can hold
constexpr uint32_t c_fifo_size_blocks = 3;


//------------------------------
//
// LMS Filter
//
//------------------------------

// must be multiple of 16
constexpr uint32_t c_num_taps = 256;

// 0 <= mu <= 2
constexpr float32_t c_mu = 0.5;






