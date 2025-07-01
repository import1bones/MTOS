#!/bin/bash
# MTOS Test Framework - Unix Shell Script
# Quick setup and testing for Linux/macOS users

set -e

echo "MTOS Test Framework for Unix/Linux/macOS"
echo "========================================"

# Check if Python is available
if ! command -v python3 &> /dev/null; then
    echo "Error: Python 3 not found. Please install Python 3.6+"
    exit 1
fi

# Run setup verification
echo "Running setup verification..."
python3 setup_tests.py
if [ $? -ne 0 ]; then
    echo "Setup verification failed!"
    exit 1
fi

# Check if user wants to build and test
echo
read -p "Build and run tests? (y/n): " choice
case "$choice" in 
  y|Y ) 
    echo "Building MTOS..."
    make clean
    make all
    
    if [ $? -ne 0 ]; then
        echo "Build failed!"
        exit 1
    fi
    
    echo
    echo "Running tests..."
    make test
    
    if [ $? -ne 0 ]; then
        echo "Tests failed!"
        exit 1
    fi
    
    echo
    echo "All tests completed successfully!"
    ;;
  * ) 
    echo "Skipping build and test."
    ;;
esac

echo
echo "Available commands:"
echo "  make run      - Run OS in QEMU"
echo "  make debug    - Run with debugger"
echo "  make test     - Run all tests"
echo
