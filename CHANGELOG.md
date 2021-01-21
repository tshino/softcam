# Change Log

All notable changes to the Softcam library will be documented in this file.

### [Unreleased]
- Fixed an issue on Debug DLL that was making an assertion failure when a console program that uses the DLL stops by pressing Ctrl+C.
    - The cause was a global destructor of a std::mutex object in softcam.dll. The destructor claimed that the mutex object is still locked. That happens because when Ctrl+C has pressed the whole program execution stops immediately no matter the state of mutex objects and then the global destructors are invoked after that.
    - This fix removes unnecessary global destructors to avoid such a scenario.
- Improved accuracy of timing control.


### [1.2] - 2021-01-18
- Added support for 32-bit camera applications [#3](https://github.com/tshino/softcam/issues/3).
    - To support both 32-bit and 64-bit camera applications, each of the DLL `softcam.dll` of both modes must be built and installed.
    - Once both DLLs are installed in a system, no matter which one your application links to or which one the camera application accesses to, they can communicate and the images are streamed between them correctly.
- Changed the layout of the `dist` directory:
    - `dist/bin/x64`   <- `dist/bin`
    - `dist/lib/x64`   <- `dist/lib`
    - `dist/bin/Win32` (NEW)
    - `dist/lib/Win32` (NEW)
- Fixed an issue on Debug DLL name confusion.
    - The example program `sender.exe` with `Debug` configuration was failing to run because the import library `softcamd.lib` wrongly tries to find `softcam.dll` (not `softcamd.dll`).
    - Now the import library is fixed to use `softcamd.dll` correctly.


### [1.1] - 2021-01-02
- Added PostBuildEvent which makes the `dist` directory that holds the header and the binaries.
- Changed the build configuration of examples to use the `dist` directory and made them independent of the library's build.
- Improved accuracy of timing calculation.
- Added a figure on top of the README.


### [1.0] - 2020-12-16
- Initial release
