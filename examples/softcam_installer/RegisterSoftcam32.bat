@echo off

pushd %~dp0
set INSTALLER=Win32\Release\softcam_installer.exe
set TARGET=..\..\dist\bin\Win32\softcam.dll

echo ##############################################################
echo Softcam Installer (softcam_installer.exe) will install Softcam
echo (softcam.dll) to your system.
echo ##############################################################
echo.

%INSTALLER% register %TARGET%

if %ERRORLEVEL% == 0 (
  echo.
  echo Successfully done.
  echo.
) else (
  echo.
  echo The process has been canceled or failed.
  echo.
)
popd
pause
