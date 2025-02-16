#!/bin/bash

# Stop on any error
set -e

# Build directory
BUILD_DIR=build

# Clean old build
if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir "$BUILD_DIR"
cd "$BUILD_DIR"

# Run CMake
cmake ..

# Build the project
make

# Run the executable
./bin/rtsp_server_dynamic "Initial Text" "file_input.txt" "rtsp://127.0.0.1:8554"


/*


#!/bin/bash

# Replace 777 permissions with read, write, and remove only
TARGET_DIR="/path/to/your/directory"

chmod -R u=rw,g=rw,o=rw "$TARGET_DIR"
echo "Updated permissions for $TARGET_DIR"

*/