#pragma once

#include <ao/ao.h>
#include <mpg123.h>

#include <vector>
#include <string>

class AudioStream
{
public:
    void FromFile(std::string filename)
    {
        int status;
        m_handle = mpg123_new(NULL, &status);

        // TODO: error handling
        // if(status == xxx)
        // ...

        m_buffer.resize(mpg123_outblock(m_handle));
        mpg123_open(m_handle, filename.c_str());
        mpg123_getformat(m_handle, &m_rate, &m_numChannels, &m_encoding);

        m_format.bits = mpg123_encsize(m_encoding) * 8;
        m_format.rate = m_rate;
        m_format.channels = m_numChannels;
        m_format.byte_format = AO_FMT_NATIVE;
        m_format.matrix = 0;
    }
    void Play()
    {
        m_audioDevice = ao_open_live(m_audioDriver, &m_format, NULL);
        size_t status = 0;
        while(mpg123_read(m_handle, (unsigned char*)m_buffer.data(),
            m_buffer.size(), &status) == MPG123_OK)
        {
            ao_play(m_audioDevice, (char*)m_buffer.data(), status);
        }
        ao_close(m_audioDevice);
    }
    void Release()
    {

    }
    AudioStream()
    {
        ao_initialize();
        m_audioDriver = ao_default_driver_id();
        mpg123_init();
    }
    ~AudioStream()
    {
        mpg123_close(m_handle);
        mpg123_delete(m_handle);
        mpg123_exit();
        ao_shutdown();
    }
private:
    mpg123_handle* m_handle;
    std::vector<char> m_buffer;

    int m_audioDriver;
    ao_device* m_audioDevice;
    ao_sample_format m_format;
    int m_numChannels;
    int m_encoding;
    long m_rate;
};
