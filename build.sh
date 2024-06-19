#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Remove the existing build directory if it exists to ensure a clean build environment
rm -rf build

# Create a new build directory
mkdir build

# Navigate to the build directory
cd build

# Run CMake to configure the project, specifying the parent directory as the source
cmake ..

# Build the project using the generated Makefile
make