#pragma once



template <typename T>
class CircularBuffer
{
protected:
    T* buffer_start_;
    T* buffer_limit_;
    T* read_ptr_;
    T* write_ptr_;
    size_t block_size_;
    bool buffer_full_ {false};

    // error counter
    uint32_t overrun_error_ {0};
    uint32_t underrun_error_ {0};

public:
    // ctor
    CircularBuffer(size_t block_size, T buffer[], size_t num_blocks)
        : buffer_start_ {buffer}, buffer_limit_ {buffer + (block_size * num_blocks)}, read_ptr_ {buffer}, write_ptr_ {buffer}, block_size_ {block_size}
        {}

    // N = block_size * num_blocks
    template <size_t N>
    CircularBuffer(size_t block_size, T (&buffer)[N])
        : buffer_start_ {buffer}, buffer_limit_ {buffer + N}, read_ptr_ {buffer}, write_ptr_ {buffer}, block_size_ {block_size}
        {
            ASSERT((N % block_size) == 0);
        }

    size_t
    get_buffer_size() const
        {
            return (buffer_limit_ - buffer_start_);
        }

    size_t
    get_block_size() const
        {
            return block_size_;
        }

    bool
    is_empty() const
        {
            return (read_ptr_ == write_ptr_ && !buffer_full_);
        }

    bool
    is_full() const
        {
            return buffer_full_;
        }

    void
    reset()
        {
            read_ptr_ = write_ptr_ = buffer_start_;
            buffer_full_ = false;
            overrun_error_ = 0;
            underrun_error_ = 0;
        }

    uint32_t
    capture_overrun_error()
        {
            uint32_t tmp = overrun_error_;
            overrun_error_ = 0;
            return tmp;
        }

    uint32_t
    capture_underrun_error()
        {
            uint32_t tmp = underrun_error_;
            underrun_error_ = 0;
            return tmp;
        }

    uint32_t
    get_overrun_error()
        {
            return overrun_error_;
        }

    uint32_t
    get_underrun_error()
        {
            return underrun_error_;
        }


    // returns false if buffer is full
    bool
    write_sample(T sample)
        {
            if(buffer_full_)
            {
                overrun_error_++;
                return false;
            }

            // copy sample into buffer
            *write_ptr_++ = sample;                            

            // check if end of buffer is reached
            if(write_ptr_ == buffer_limit_)
            {
                write_ptr_ = buffer_start_;
            }

            if(write_ptr_ == read_ptr_)
            {
                // buffer full
                buffer_full_ = true;
            }

            return true;
        }


    // returns false if buffer is empty
    bool
    read_sample(T& sample)
        {
            if(is_empty())
            {
                // buffer empty
                underrun_error_++;
                return false;
            }

            // copy sample
            sample = *read_ptr_++;

            // check if end of buffer is reached
            if(read_ptr_ == buffer_limit_)
            {
                read_ptr_ = buffer_start_;
            }

            buffer_full_ = false;

            return true;
        }


    // output buffer must be at least of block size
    // returns false if no block can be read
    bool
    read_block(T output[])
        {
            // check if buffer has at least one block
            if(num_blocks_readable() == 0)
            {
                underrun_error_++;
                return false;
            }

            // copy data from buffer to sample_block
            for(size_t n = block_size_; n > 0; n--)
            {
                // copy sample
                *output++ = *read_ptr_++;

                // check if end of buffer is reached
                if(read_ptr_ == buffer_limit_)
                {
                    read_ptr_ = buffer_start_;
                }
            }

            buffer_full_ = false;

            return true;
        }


    bool
    write_block(const T input[])
        {
            // check if buffer has at least one writeable block
            if(num_blocks_writeable() == 0)
            {
                overrun_error_++;
                return false;
            }

            // copy data from sample_block to buffer
            for(size_t n = block_size_; n > 0; n--)
            {
                // copy sample
                *write_ptr_++ = *input++;

                // check if end of buffer is reached
                if(write_ptr_ == buffer_limit_)
                {
                    write_ptr_ = buffer_start_;
                }
            }

            if(write_ptr_ == read_ptr_)
            {
                // buffer full
                buffer_full_ = true;
            }

            return true;
        }


    // returns nullptr if less than one block is readable
    T*
    get_read_block_ptr()
        {
            // check if buffer has at least one block
            if(num_blocks_readable() == 0)
            {
                underrun_error_++;
                return nullptr;
            }

            return read_ptr_;
        }


    // increments read ptr by one block
    void
    incr_read_block_ptr()
        {
            if(is_empty())
            {
                underrun_error_++;
                return;
            }

            read_ptr_ += block_size_;
            // check if end of buffer is reached
            if(read_ptr_ == buffer_limit_)
            {
                read_ptr_ = buffer_start_;
            }

            buffer_full_ = false;
        }


    // returns nullptr if less than one block is writeable
    T*
    get_write_block_ptr()
        {
            // check if buffer has at least one writeable block
            if(num_blocks_writeable() == 0)
            {
                overrun_error_++;
                return nullptr;
            }

            return write_ptr_;
        }


    // increments write ptr by one block
    void
    incr_write_block_ptr()
        {
            if(is_full())
            {
                overrun_error_++;
                return;
            }

            write_ptr_ += block_size_;
            // check if end of buffer is reached
            if(write_ptr_ == buffer_limit_)
            {
                write_ptr_ = buffer_start_;
            }

            if(write_ptr_ == read_ptr_)
            {
                // buffer full
                buffer_full_ = true;
            }
        }


    bool
    discard_block()
        {
            // check if buffer has at least one block
            if(num_blocks_readable() > 0)
            {
                for(size_t n = block_size_; n > 0; n--)
                {
                    read_ptr_++;
                    
                    // check if end of buffer is reached
                    if(read_ptr_ == buffer_limit_)
                    {
                        read_ptr_ = buffer_start_;
                    }
                }

                buffer_full_ = false;

                return true;
            }

            return false;
        }


    bool
    discard_sample()
        {
            // check if buffer has at least one sample
            if(!is_empty())
            {
                read_ptr_++;
                
                // check if end of buffer is reached
                if(read_ptr_ == buffer_limit_)
                {
                    read_ptr_ = buffer_start_;
                }

                buffer_full_ = false;
                
                return true;
            }

            return false;
        }

    bool
    fill_block(T value)
        {
            // check if buffer has at least one writeable block
            if(num_blocks_writeable() == 0)
            {
                overrun_error_++;
                return false;
            }

            for(size_t n = block_size_; n > 0; n--)
            {
                *write_ptr_++ = value;

                // check if end of buffer is reached
                if(write_ptr_ == buffer_limit_)
                {
                    write_ptr_ = buffer_start_;
                }
            }

            if(write_ptr_ == read_ptr_)
            {
                // buffer full
                buffer_full_ = true;
            }

            return true;
        }


    size_t
    num_blocks_readable()
        {
            return ( num_samples_readable() / block_size_ );
        }


    size_t
    num_blocks_writeable()
        {
            return ( num_samples_writeable() / block_size_ );
        }


    size_t
    num_samples_readable()
        {
            if(buffer_full_)
            {
                return get_buffer_size();
            }

            if(write_ptr_ >= read_ptr_)
            {
                return (write_ptr_ - read_ptr_);
            }
            else
            {
                return ( get_buffer_size() - (read_ptr_ - write_ptr_) );
            }
        }


    size_t
    num_samples_writeable()
        {
            if(buffer_full_)
            {
                return 0;
            }

            if(write_ptr_ >= read_ptr_)
            {
                return ( get_buffer_size() - (write_ptr_ - read_ptr_) );
            }
            else
            {
                return (read_ptr_ - write_ptr_);
            }
        }
};

