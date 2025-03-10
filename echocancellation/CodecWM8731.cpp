#include "global.h"
#include "CodecWM8731.h"




CodecWM8731* g_codec;





CodecWM8731::CodecWM8731()
{
}


void 
CodecWM8731::create_instance()
{
    static CodecWM8731 codec_instance;
    g_codec = &codec_instance;
}


void
CodecWM8731::init()
{
    // Activate I2C interface to CODEC and configure CODEC registers
	CodecInit(c_samp_freq, c_audio_source);
}



