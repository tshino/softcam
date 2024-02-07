# Change Log

All notable changes to the Softcam library will be documented in this file.

### [1.8] - 2024-02-08
- Added `scIsConnected()` method to API to make application connection waiting more flexible by @jekyll2014. (thanks!) [#51](https://github.com/tshino/softcam/pull/51)
- Added corresponding `is_connected()` method to the python_binding example.
- Updated solution file version from Visual Studio 2019 to Visual Studio 2022. [#45](https://github.com/tshino/softcam/pull/45)
- Added `pushd`/`popd` to example installer BAT files to work correctly even if launched from another directory. [#48](https://github.com/tshino/softcam/pull/48)

### [1.7] - 2023-12-02
- Bumped NuGet package version of Google Test.
- Changed the primary target compiler from Visual Studio 2019 to Visual Studio 2022. [#44](https://github.com/tshino/softcam/pull/44)
    - Have set TargetName property explicitly instead of letting it use the default ($(ProjectName)) for each project to prevent output file names from changing even if project file names change.
    - Duplicated all existing project files with _vs2019 suffix added to maintain VS2019 support.
        - For example, `softcam_vs2019.sln` can be used as same as the previous `softcam.sln`. All the output files keep the same name as before.
    - Updated the GitHub Action workflow files to let them keep using the latest solution file (`softcam.sln`).
    - Retargeted all project files (except `*_vs2019`) to use the VS2022 toolset.
- Refactoring:
    - Named namespaces of each test code for better visualization on Visual Studio. [#42](https://github.com/tshino/softcam/pull/42)

### [1.6] - 2023-09-10
- Fixed a deadlock issue. [#39](https://github.com/tshino/softcam/pull/39)

### [1.5] - 2023-03-07
- Updated the python_binding example:
    - Fixed wrong argument name in the python_binding example. [#16](https://github.com/tshino/softcam/pull/16)
    - Added CI build and test of the python_binding example. [#17](https://github.com/tshino/softcam/issues/17)
    - Updated Pybind11 version. [#24](https://github.com/tshino/softcam/pull/24)

### [1.4] - 2022-12-18
- Added Python binding of this library. [#12](https://github.com/tshino/softcam/issues/12)
- Improved error handling.


### [1.3] - 2021-02-01
- Improved accuracy of timing control.
- Fixed an issue on Debug DLL.
    - The `sender` example program was making an assertion failure when it stops with Ctrl+C pressed by the user.
    - The cause of the assertion failure is a global destructor for a global std::mutex object in softcam.dll. The assertion in the destructor claimed that the mutex object is still locked. That happens because when Ctrl+C has pressed the whole program execution stops immediately no matter the state of mutex objects and then the global destructors are invoked after that.
    - This fix removes unnecessary global destructors to avoid such a scenario.


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
