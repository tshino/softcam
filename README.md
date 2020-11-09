# Softcam

This is a library to make a virtual webcam on Windows.


## How it works

This software is a DLL which implements a DirectShow filter of Video Input Device category.
Once it is installed to the system, any camera application which is using DirectShow API will recognize the filter as a camera-like device attached to the system.
And thankfully many of camera applications running on Windows use DirectShow API to access camera devices.

This DLL provides Softcam Sender API too. Using this API, your application can create a virtual webcam instance and send arbitrary images as a video stream to the camera application.

Your code will look like this:

```
auto camera = scCreateCamera(320, 240, 60);
for (;;)
{
    uint8_t image[320 * 240 * 3];
    DrawSomething(image);
    scSendFrame(camera, image);
}
```
.

## How to build and run

Following software is required.

- Visual Studio 2019 (installed with Desktop development with C++ and Windows 10 SDK)

The procedure is as follows.

1. Open `softcam.sln`, choose configuration `Release` and build the solution. If it successfully builds, you will get `softcam.dll` in `x64/Release` directory.
2. Open `examples/softcam_installer/softcam_installer.sln`, choose configuration `Release` and build the solution. Then you will get `softcam_installer.exe` in `examples/softcam_installer/x64/Release` directory.
3. Then you register `softcam.dll` to your system by double clicking the `RegisterSoftcam.bat` in the `examples/softcam_installer` directory. Note since this process requires Windows administrator permissions, `softcam_installer.exe` will request the permissions to the user through UAC. You can also unregister the DLL using `UnregisterSoftcam.bat` with the same manner.
4. Open `examples/sender/sender.sln`, choose configuration `Release` and build the solution.
5. Run `sender.exe`. If it worked, you will see the message '`Ok. Softcam is now active.`' in the console. This means that you got a new virtual webcam running on the system.
6. To see the video stream which is being sent from the `sender.exe`, you may try my web application Compare ([https://tshino.github.io/compare/compare.html](https://tshino.github.io/compare/compare.html)) on your browser. This is an image comparison tool which is written in JavaScript, and it can take a capture from a webcam. Open the above link and click the camera icon which appeares on the side bar. If it shows a different camera image, you need to change the settings of your browser to select '`DirectShow Softcam`' as main camera device. In case of Chrome or Edge, the camera icon on the address bar is a shortcut to the camera settings.


## License

[MIT](LICENSE).

Note that the Base Classes libary (in [src/baseclasses](src/baseclasses) directory) is a part of [Microsft Windows Samples](https://github.com/microsoft/Windows-classic-samples) (Base Classes is in [Samples/Win7Samples/multimedia/directshow/baseclasses](https://github.com/microsoft/Windows-classic-samples/tree/master/Samples/Win7Samples/multimedia/directshow/baseclasses) directory) which is also provided under MIT license.
