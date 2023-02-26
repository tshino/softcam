@echo off
@REM Call this batch with your target python environment active
@REM to launch Visual Studio with appropriate environment variable values.
@REM - PYTHON_INCLUDE_DIR
@REM - PYTHON_LIBS_DIR
setlocal

pushd %~dp0

call _GetPythonPath.bat

start python_binding.sln

popd
endlocal
