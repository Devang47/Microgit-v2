#!/bin/bash
# Simple build script for MicroGit

# Make the script executable if it isn't already
if [ ! -x "$0" ]; then
  chmod +x "$0"
fi

# Create build directory if it doesn't exist and ensure proper permissions
mkdir -p build
if [ ! -w "build" ]; then
  echo "Fixing permissions on build directory..."
  chmod u+w build
fi

# Change to build directory
cd build

# Configure with CMake (using absolute paths to avoid permission issues)
BUILD_DIR="$(pwd)"
SOURCE_DIR="$(cd .. && pwd)"
echo "Building from source: $SOURCE_DIR"
echo "Building in: $BUILD_DIR"

# Clean any previous failed build configurations
echo "Cleaning previous build files..."
rm -rf CMakeCache.txt CMakeFiles

# Force using the header-only approach for JSON to avoid compatibility issues
echo "Using header-only approach for JSON library..."
mkdir -p _deps/json-src/include/nlohmann
if [ ! -f _deps/json-src/include/nlohmann/json.hpp ]; then
  echo "Downloading json.hpp..."
  curl -s -L https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp -o _deps/json-src/include/nlohmann/json.hpp
  if [ $? -ne 0 ]; then
    echo "Failed to download json.hpp"
    exit 1
  fi
  echo "Downloaded json.hpp successfully."
fi

# Run CMake with explicit source and build directories
echo "Running CMake..."
cmake -DCMAKE_BUILD_TYPE=Debug "$SOURCE_DIR"

# Determine number of CPU cores for parallel build
if [ "$(uname)" == "Darwin" ]; then
  # macOS
  CORES=$(sysctl -n hw.ncpu)
else
  # Linux and others
  CORES=$(nproc 2>/dev/null || echo "2")
fi

# Build
echo "Building with $CORES cores..."
cmake --build . -- -j$CORES

# Check if build was successful
if [ $? -eq 0 ]; then
  echo "Build successful! Executable is at: $BUILD_DIR/microgit"
  echo "You can run it with: ./microgit"
  echo ""
  echo "Example commands:"
  echo "  ./microgit init          - Initialize a new repository"
  echo "  ./microgit --help        - Show help information"
else
  echo "Build failed!"
  echo "Try manual compilation steps:"
  echo "  cd $BUILD_DIR"
  echo "  g++ -std=c++23 -I$SOURCE_DIR -I$BUILD_DIR/_deps/json-src/include -o microgit $SOURCE_DIR/main.cpp $SOURCE_DIR/cmd/root.cpp $SOURCE_DIR/cmd/init.cpp $SOURCE_DIR/utils/main.cpp -lcrypto"
fi
