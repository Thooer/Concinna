@echo off
title File Collector

echo Starting file collection...
echo.

cd /d "%~dp0"

python move_files.py

if errorlevel 1 (
    echo.
    echo Script execution failed!
) else (
    echo.
    echo Script execution completed!
)

echo.
pause