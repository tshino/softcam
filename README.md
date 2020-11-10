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
3. Then you register the DLL `softcam.dll` to your system by double clicking the `RegisterSoftcam.bat` in the `examples/softcam_installer` directory. This batch file simply launches `softcam_installer.exe` with the path of the DLL as a command line option. This step, in other words installing this software, does not actually copy any files, instead, it writes the absolute path of the DLL on the registry using appropriate API. Note since this process requires administrator permissions, `softcam_installer.exe` will request the permissions to the user through UAC. You can also unregister the DLL using `UnregisterSoftcam.bat` with the same manner.
4. Open `examples/sender/sender.sln`, choose configuration `Release` and build the solution. Then you get `sender.exe` in `examples/sender/x64/Release' directory.
5. Run `sender.exe`. If it runs successfully, you will see a message '`Ok. Softcam is now active.`' in the console. Now you've got a new virtual webcam running on your system.
6. To see the video stream which is being produced by the `sender.exe`, you can use any camera application. If you don't have a preference at this moment, I recommend you to try my web application `Compare` ([https://tshino.github.io/compare/compare.html](https://tshino.github.io/compare/compare.html)). This is an image comparison tool written in JavaScript, and it has the capability of taking a capture from a webcam. Open the above link, find the camera icon appears on the sidebar, and click it. If it shows a different camera image or fails to connect to the camera, you may need to look at the settings of your browser. Make sure to let the browser use '`DirectShow Softcam`' as a camera device. If it doesn't appear in the list of available cameras, you may need to reboot the browser (or Windows). In most browsers, a camera icon appears in the address bar, which is a shortcut to the camera settings.


## License

[MIT](LICENSE).

Note that the Base Classes libary (in [src/baseclasses](src/baseclasses) directory) is a part of [Microsft Windows Samples](https://github.com/microsoft/Windows-classic-samples) (Base Classes is in [Samples/Win7Samples/multimedia/directshow/baseclasses](https://github.com/microsoft/Windows-classic-samples/tree/master/Samples/Win7Samples/multimedia/directshow/baseclasses) directory) which is also provided under MIT license.
