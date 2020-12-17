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

        The `width` argument and the `height` argument specify the dimension
        of the camera image. The width and the height should be a positive
        number and be a multiple of four.

        The `framerate` argument is an optional argument that specifies the
        reference framerate of the camera. The default framerate is 60.
        If the value of the `framerate` argument is 0, the interval of each
        frame is not constrained.

        If this function succeeds, it returns the handle of a new virtual
        camera instance, otherwise, it returns a null pointer.

        This function fails if another instance already exists in the system.

        The new instance created by this function should be deleted with
        the `scDeleteCamera` function when it no longer is used.
        If the caller process terminates without calling the `scDeleteCamera`
        function, the virtual camera intance and associated resources are
        deleted automatically.
    */
    scCamera    scCreateCamera(int width, int height, float framerate = 60.0f);

    /*
        This function deletes a virtual camera instance.
    */
    void        scDeleteCamera(scCamera camera);

    /*
        This function sends a new frame of a virtual camera.

        If the framerate set to the virtual camera is not zero, this
        function tries to make the timing to deliver the new image ideal
        as much as possible by sleeping for an appropriate time inside the
        function.

        If the framerate set to the virtual camera is zero, this function
        sends the new image immediately and does not control that timing.
        This is useful if the application uses another source stream such
        as actual webcams.
    */
    void        scSendFrame(scCamera camera, const void* image_bits);

    /*
        This function waits for an application to connect to a virtual camera.

        If the `timeout` argument is greater than 0, this function timeouts
        after the specified time if no application accesses the virtual camera.
    */
    bool        scWaitForConnection(scCamera camera, float timeout = 0.0f);
}
