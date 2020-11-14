#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include "Misc.h"


namespace softcam {


using std::uint64_t;
using std::uint32_t;
using std::uint16_t;


/// Shared frame buffer between processes (sender and receiver)
class FrameBuffer
{
 public:
    static FrameBuffer create(
                        int             width,
                        int             height,
                        float           framerate = 0.0f);
    static FrameBuffer open();

    explicit operator bool() const { return handle() != nullptr; }

    void*           handle() const;
    int             width() const;
    int             height() const;
    float           framerate() const;
    uint64_t        frameCounter() const;
    bool            active() const;
    bool            connected() const;

    void            deactivate();
    void            write(const void* image_bits);
    void            transferToDIB(void* image_bits, uint64_t* out_frame_counter);
    bool            waitForNewFrame(uint64_t frame_counter, float time_out = 0.5f);

    void            release();

 private:
    struct Header;

    mutable NamedMutex      m_mutex;
    SharedMemory            m_shmem;
    std::shared_ptr<void>   m_watchdog_restarter;
    uint8_t                 m_watchdog_last_value = 0;
    Timer                   m_watchdog_timer;

    explicit FrameBuffer(const char* mutex_name) : m_mutex(mutex_name) {}

    Header*         header();
    const Header*   header() const;

    static bool     checkDimensions(
                        int width,
                        int height);
    static uint32_t calcMemorySize(
                        uint16_t width,
                        uint16_t height);
};


} //namespace softcam
