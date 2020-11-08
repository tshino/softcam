#include "SenderAPI.h"

#include <mutex>
#include <memory>

#include "FrameBuffer.h"
#include "Misc.h"


namespace {

struct Camera
{
    softcam::FrameBuffer    m_frame_buffer;
    softcam::Timer          m_timer;
};

std::mutex              s_mutex;
std::unique_ptr<Camera> s_camera;

} //namespace


namespace softcam {
namespace sender {

CameraHandle    CreateCamera(int width, int height, float framerate)
{
    std::lock_guard<std::mutex> lock(s_mutex);

    if (!s_camera)
    {
        if (auto fb = FrameBuffer::create(width, height, framerate))
        {
            s_camera.reset(new Camera{ fb, Timer() });
            return fb.handle();
        }
    }
    return nullptr;
}

void            DeleteCamera(CameraHandle camera)
{
    std::lock_guard<std::mutex> lock(s_mutex);

    if (s_camera && s_camera->m_frame_buffer.handle() == camera)
    {
        s_camera->m_frame_buffer.deactivate();
        s_camera.reset();
    }
}

void            SendFrame(CameraHandle camera, const void* image_bits)
{
    std::lock_guard<std::mutex> lock(s_mutex);

    if (s_camera && s_camera->m_frame_buffer.handle() == camera)
    {
        auto framerate = s_camera->m_frame_buffer.framerate();
        auto frame_counter = s_camera->m_frame_buffer.frameCounter();

        // To deliver frames in the regular period, we sleep here a bit
        // before we deliver the new frame if it's not the time yet.
        // If it's already the time, we deliver it immediately and
        // let the timer keep running so that if the next frame comes
        // in time the constant delivery recovers.
        // However if the delay grew too much (greater than 50 percent
        // of the period), we reset the timer to avoid continuing
        // irregular delivery.
        if (0.0f < framerate && 0 < frame_counter)
        {
            auto ref_delta = 1.0f / framerate;
            auto time = s_camera->m_timer.get();
            if (time < ref_delta)
            {
                Timer::sleep(ref_delta - time);
            }
            if (time < ref_delta * 1.5f)
            {
                s_camera->m_timer.rewind(ref_delta);
            }
            else
            {
                s_camera->m_timer.reset();
            }
        }

        s_camera->m_frame_buffer.write(image_bits);
    }
}

bool            WaitForConnection(CameraHandle camera, float timeout)
{
    std::lock_guard<std::mutex> lock(s_mutex);

    if (s_camera && s_camera->m_frame_buffer.handle() == camera)
    {
        Timer timer;
        while (!s_camera->m_frame_buffer.connected())
        {
            if (0.0f < timeout && timeout <= timer.get())
            {
                return false;
            }
            Timer::sleep(0.001f);
        }
        return true;
    }
    return false;
}

} //namespace sender
} //namespace softcam
