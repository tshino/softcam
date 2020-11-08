#pragma once


//
// Softcam Sender API
//

extern "C"
{
    using scCamera = void*;

    scCamera    scCreateCamera(int width, int height, float framerate = 60.0f);
    void        scDeleteCamera(scCamera camera);
    void        scSendFrame(scCamera camera, const void* image_bits);
    bool        scWaitForConnection(scCamera camera, float timeout = 0.0f);
}
