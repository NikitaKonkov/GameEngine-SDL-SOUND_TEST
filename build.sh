#!/bin/bash

echo "Building Engine project..."

# Default settings
USE_AVX512=OFF
CLEAN_BUILD=OFF

# Check if CMake is installed
if ! command -v cmake &> /dev/null; then
    echo "Error: CMake is not installed or not in PATH"
    echo "Please install CMake using your package manager:"
    echo "    sudo apt install cmake    # For Debian/Ubuntu"
    echo "    sudo dnf install cmake    # For Fedora"
    echo "    sudo pacman -S cmake      # For Arch Linux"
    exit 1
fi

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --avx512)
            USE_AVX512=ON
            shift
            ;;
        --avx2)
            USE_AVX512=OFF
            shift
            ;;
        --clean)
            CLEAN_BUILD=ON
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: ./build.sh [--avx512] [--avx2] [--clean]"
            exit 1
            ;;
    esac
done

# Display build info
echo "[BUILD INFO] Using instruction set: $USE_AVX512"
if [ "$USE_AVX512" = "ON" ]; then
    echo "[BUILD INFO] AVX-512 instructions will be enabled"
else
    echo "[BUILD INFO] AVX-2 instructions will be used (default)"
fi

# Create build directory if it doesn't exist
mkdir -p build

# Clean build if requested
if [ "$CLEAN_BUILD" = "ON" ]; then
    echo "Cleaning build directory..."
    rm -rf build
    mkdir -p build
fi

# Navigate to build directory
cd build

# Configure with CMake (using Ninja for faster builds in MSYS2)
echo "Configuring with CMake..."
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=TRUE -DUSE_AVX512=$USE_AVX512 -G "Ninja" ../Engine

# Build the project
echo "Building project..."
cmake --build . --config Release

# Check if build was successful
BUILD_RESULT=$?
if [ $BUILD_RESULT -ne 0 ]; then
    echo "Build failed with error code $BUILD_RESULT"
    exit $BUILD_RESULT
fi

echo "Build successful!"
cd ..

# Run the executable (adjust the path as needed for MSYS2/UCRT64)
echo "Running the application..."
./build/gameengine

echo "Done!"

# Usage instructions:
# ./build.sh           # Build with AVX2 (default)
# ./build.sh --avx512  # Build with AVX512
# ./build.sh --clean   # Clean build with default AVX2
# ./build.sh --clean --avx512  # Clean build with AVX512