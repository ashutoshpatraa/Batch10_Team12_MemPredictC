@echo off
if not exist build\MemPredictC.exe (
    echo MemPredictC.exe is missing. Run build.bat first.
    exit /b 1
)
build\MemPredictC.exe
