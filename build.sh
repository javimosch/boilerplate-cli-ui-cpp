#!/bin/bash
# Build script for boilerplate-cli-ui-cpp

set -e

APP_NAME="boilerplate-cli-ui-cpp"

echo "Building ${APP_NAME}..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build
make -j$(nproc)

echo ""
echo "Built: build/${APP_NAME}"
ls -lh ${APP_NAME}

echo ""
echo "Usage:"
echo "  ./build/${APP_NAME} start           # Start server with UI"
echo "  ./build/${APP_NAME} start -p 3000   # Start on custom port"
echo "  ./build/${APP_NAME} version         # Show version"
