#pragma once


//
// Softcam Sender API
//

extern "C"
{
    using scCamera = void*;

    /*
        This function creates a virtual camera instance.

        Camera applications running on the system will be able to find the
        new virtual camera through DirectShow API.

        `width` and `height` specify the dimension of the camera image.
        The width and height should be a positive number and be a multiple
        of four.

        `framerate` is an optional argument that specifies the reference
        framerate of the camera. The default framerate is 60.
        If the value of `framerate` is 0, the interval of each frame is not
        constrained.

        If this function succeeded it returns a handle of the new virtual
        camera instance, otherwise, it returns a null pointer.

        This function fails if another instance already exists in the system.

        The new instance created by this function should be deleted with
        `scDeleteCamera` when it no longer is used.
        If the caller process has terminated without calling `scDeleteCamera`,
        the virtual camera and associated resources are deleted automatically.
    */
    scCamera    scCreateCamera(int width, int height, float framerate = 60.0f);

    /*
        This function deletes a virtual camera instance.
    */
    void        scDeleteCamera(scCamera camera);

    /*
        This function sends a new frame of a virtual camera.
    */
    void        scSendFrame(scCamera camera, const void* image_bits);

    /*
        This function waits for an application to connect to a virtual camera.
    */
    bool        scWaitForConnection(scCamera camera, float timeout = 0.0f);
}
