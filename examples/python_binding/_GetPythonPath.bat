@echo off

python -c "import sys;print(sys.prefix)" > _python_dir.txt
set /p PYTHON_DIR=<_python_dir.txt
del _python_dir.txt

set PYTHON_INCLUDE_DIR=%PYTHON_DIR%\include
set PYTHON_LIBS_DIR=%PYTHON_DIR%\libs
