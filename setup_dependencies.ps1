# Create directories
New-Item -ItemType Directory -Path "external/include", "external/lib" -Force

# Clone raylib
if (!(Test-Path "raylib_src")) {
    git clone --depth 1 https://github.com/raysan5/raylib.git raylib_src
}

# Build raylib using CMake (Targets Visual Studio 2022)
cd raylib_src
mkdir build
cd build
cmake .. -DPLATFORM=Desktop -DBUILD_EXAMPLES=OFF -DBUILD_GAMES=OFF
cmake --build . --config Release

# Copy files to our external folder
Copy-Item "..\src\raylib.h" "..\..\external\include\"
Copy-Item "..\src\raymath.h" "..\..\external\include\"
Copy-Item "..\src\rlgl.h" "..\..\external\include\"
Copy-Item ".\raylib\Release\raylib.lib" "..\..\external\lib\"

cd ..\..\
Write-Host "Raylib built for Visual Studio and installed to .\external\" -ForegroundColor Green