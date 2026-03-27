#!/bin/bash

# Create external directory structure
mkdir -p external/include external/lib

# Clone raylib master branch if it doesn't exist
if [ ! -d "raylib_src" ]; then
    git clone --depth 1 https://github.com/raysan5/raylib.git raylib_src
fi

# Build raylib
cd raylib_src/src
make PLATFORM=PLATFORM_DESKTOP

# Move files to our external folder
cp raylib.h ../../external/include/
cp raymath.h ../../external/include/
cp rlgl.h ../../external/include/
cp libraylib.a ../../external/lib/

cd ../../
echo "Raylib built and installed to ./external/"