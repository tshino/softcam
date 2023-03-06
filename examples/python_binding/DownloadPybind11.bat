@echo off
@REM Run this batch once before building this project.
@REM This batch requires internet connection to download the Pybind11 archive.
setlocal

set PYBIND11_VER=2.10.3

pushd %~dp0
if not exist pybind11 (
  curl -L -o pybind11.zip https://github.com/pybind/pybind11/archive/refs/tags/v%PYBIND11_VER%.zip || goto :fail
  tar -xf pybind11.zip || goto :fail
  del pybind11.zip
  rename pybind11-%PYBIND11_VER% pybind11 || goto :fail
  echo OK: Pybind11 has been downloaded successfully.
) else (
  echo OK: Pybind11 has already been downloaded.
)
popd
pause
goto :end

:fail
popd
echo Error! Downloading Pybind11 failed.
pause

:end
endlocal
