# Minetale

## Dependencies

- CMake 3.10+
- C++17 compiler
- GLEW
- OpenGL

GLFW, GLM, and Dear ImGui are bundled under `external/`.

### Linux

**Arch:**
```
sudo pacman -S cmake glew
```

**Ubuntu/Debian:**
```
sudo apt install cmake libglew-dev
```

### macOS

Install [Homebrew](https://brew.sh), then:
```
brew install cmake glew
```

### Windows

Install [MSYS2](https://www.msys2.org), then from the MSYS2 MinGW64 shell:
```
pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-glew mingw-w64-x86_64-gcc
```

Alternatively, install [Visual Studio](https://visualstudio.microsoft.com) (with the C++ workload) and [vcpkg](https://vcpkg.io), then:
```
vcpkg install glew
```

## Build

### Linux / macOS

```
cmake -B build -S .
cmake --build build -j$(nproc)
```

### Windows (MSYS2 MinGW64 shell)

```
cmake -B build -S . -G "MinGW Makefiles"
cmake --build build -j$(nproc)
```

### Windows (Visual Studio)

```
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=<path\to\vcpkg>\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
```

## Run

### Linux

```
LD_LIBRARY_PATH=build ./build/main
```

### macOS

```
DYLD_LIBRARY_PATH=build ./build/main
```

### Windows

```
.\build\main.exe
```