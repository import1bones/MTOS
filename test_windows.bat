@echo off
REM MTOS Test Framework - Windows Batch Script
REM Quick setup and testing for Windows users

echo MTOS Test Framework for Windows
echo ===================================

REM Check if Python is available
python --version >nul 2>&1
if errorlevel 1 (
    echo Error: Python not found. Please install Python 3.6+
    pause
    exit /b 1
)

REM Run setup verification
echo Running setup verification...
python setup_tests.py
if errorlevel 1 (
    echo Setup verification failed!
    pause
    exit /b 1
)

REM Check if user wants to build and test
echo.
set /p choice="Build and run tests? (y/n): "
if /i "%choice%"=="y" (
    echo Building MTOS...
    make clean
    make all
    
    if errorlevel 1 (
        echo Build failed!
        pause
        exit /b 1
    )
    
    echo.
    echo Running tests...
    make test
    
    if errorlevel 1 (
        echo Tests failed!
        pause
        exit /b 1
    )
    
    echo.
    echo All tests completed successfully!
) else (
    echo Skipping build and test.
)

echo.
echo Available commands:
echo   make run      - Run OS in QEMU
echo   make debug    - Run with debugger
echo   make test     - Run all tests
echo.
pause
