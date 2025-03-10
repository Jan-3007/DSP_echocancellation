#pragma once



class I2S_DSTC
{
protected:
    // TX buffer
    uint32_t tx_buffer_space_[c_block_size * c_fifo_size_blocks];
    CircularBuffer<uint32_t> tx_buffer_{c_block_size, tx_buffer_space_};

    // RX buffer
    uint32_t rx_buffer_space_[c_block_size * c_fifo_size_blocks];
    CircularBuffer<uint32_t> rx_buffer_{c_block_size, rx_buffer_space_};

    // DSTC descriptors
    stc_dstc_des0123456_t dstc_desc_[2];
    stc_dstc_des0123456_t& tx_desc_ {dstc_desc_[0]};
    stc_dstc_des0123456_t& rx_desc_ {dstc_desc_[1]};


public:
    // ctor
    I2S_DSTC();

    static
    void
    create_instance();

    void
    init();

    void 
    start();

    bool
    write_tx_block(const uint32_t input[]);

    bool
    read_rx_block(uint32_t output[]);

    bool 
    read_rx_block_float(
        float32_t l_channel[c_block_size], 
        float32_t r_channel[c_block_size]
        );

    bool 
    write_tx_block_float(
        const float32_t l_channel[c_block_size], 
        const float32_t r_channel[c_block_size]
        );

    // send channel data to L+R
    bool 
    write_tx_block_float(
        const float32_t channel[c_block_size]
        );

    struct Errors
    {
        uint32_t tx_buffer_overrun;
        uint32_t tx_buffer_underrun;
        uint32_t rx_buffer_overrun;
        uint32_t rx_buffer_underrun;
    };

    void
    capture_errors(Errors& errors);


protected:
    void
    init_dstc();

    void
    init_i2s();


    void
    isr_tx();

    void
    isr_rx();

    static
    void
    isr_tx_static();

    static
    void
    isr_rx_static();

};



extern I2S_DSTC* g_i2s_dstc;


