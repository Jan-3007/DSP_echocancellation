#pragma once






class AEC
{
protected:
    // LMS filter
    LMSFilter lms_;

    // for LED and error check
    uint32_t block_counter {0};

    // flag for de-/activating the AEC
    bool aec_active_ {true};
    bool button_pressed {false};

    // primary input buffer
    float32_t left_input_block_[c_block_size];
    float32_t right_input_block_[c_block_size];

    // buffer for delaying the reference signal
    // this compensates the significant delay caused by the USB microphone
    float32_t delay_buffer_space_[c_block_size * (c_delay_blocks + 1)];
    CircularBuffer<float32_t> delay_buffer_{c_block_size, delay_buffer_space_};

    // primary output buffer
    float32_t left_output_block_[c_block_size];
    float32_t right_output_block_[c_block_size];


public:
    // ctor
    AEC();

    void
    init();

    void
    run();

    static
    void
    create_instance();

protected:
    void
    update_aec_flag();

    void
    process_aec();

    void
    passthrough();

    void
    check_errors();

    void
    update_led();    
};


extern AEC* g_aec;

