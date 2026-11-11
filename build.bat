@echo off
setlocal

set "GCC=gcc"
where gcc >nul 2>nul
if errorlevel 1 if exist C:\MinGW\bin\gcc.exe set "GCC=C:\MinGW\bin\gcc.exe"
if "%GCC%"=="gcc" (
    echo ERROR: MinGW GCC was not found on PATH.
    echo Install MinGW-w64 or add its bin directory to PATH, then retry.
    exit /b 1
)
if "%GCC%"=="C:\MinGW\bin\gcc.exe" set "PATH=C:\MinGW\bin;%PATH%"

for /f "tokens=*" %%V in ('"%GCC%" --version 2^>^&1') do (
    echo %%V
    goto :compiler_checked
)
:compiler_checked

if not exist build mkdir build
if not exist release mkdir release

echo Building CLI fallback...
"%GCC%" -std=c11 -Wall -Wextra -Wpedantic -O2 cli_main.c main.c memory.c predictor.c -o build\MemPredictC_CLI.exe
if errorlevel 1 (
    echo ERROR: CLI compilation failed.
    exit /b 1
)

echo Building native Windows dashboard...
"%GCC%" -std=c11 -Wall -Wextra -Wpedantic -O2 gui_main.c main.c ui.c memory.c predictor.c -o build\MemPredictC.exe -luser32 -lgdi32
if errorlevel 1 (
    echo ERROR: GUI compilation failed.
    exit /b 1
)

copy /Y build\MemPredictC.exe release\MemPredictC.exe >nul
copy /Y build\MemPredictC_CLI.exe release\MemPredictC_CLI.exe >nul
echo Build complete: build\MemPredictC.exe
echo The GUI uses only the Windows API and has no runtime DLL dependency.
exit /b 0
