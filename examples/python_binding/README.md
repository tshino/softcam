# Python Binding for Softcam

This `python_binding` project enables Softcam library to be used from Python scripts.

Your python code will look like this:

```
import softcam

cam = softcam.camera(320, 240, 60)
while True:
    img = draw_something()
    cam.send_frame(img)
```

## How to build it

1. Build Softcam library according to [README.md](../../README.md#how-to-build-the-library). You get `softcam.dll` in the `dist` directory at the root of this repository.
2. Run `DownloadPybind11.bat`. This batch downloads Pybind11 archive and expands it to be used in the following build of binding.
3. In a Command Prompt session, activate your target Python environment.
4. Run `LaunchVisualStudio.bat` in the environment. This batch prepares environment variables that point the directory of the Python environment and launch Visual Studio for the solution file `pythin_binding.sln` with that environment variables.
5. On the Visual Studio, build solution with `Release`|`x64` configuration. You will get `softcam.pyd` in the `x64/Release` directory.

Then you can run `simple_usage.py`. Note that this script requires Numpy and OpenCV installed in your Python environment.

Bring `softcam.pyd` and `softcam.dll` together to anywhere you want to use Softcam in python scripts.

Note:

- The color component order should be BGR, not RGB.
- Different version of Python interpreter needs different build of `softcam.pyd`.
    - For example, if you have built `softcam.pyd` with Python 3.9, trying to use it on Python 3.8 will fail with `ImportError`.
- You can not change the filename of `softcam.pyd` after build.
    - If you rename it to `foo.pyd` and run `import foo` or `import softcam` in your Python code, either will fail with the `ImportError`.
    - This is because the name of the module is embedded in the file in build time.
