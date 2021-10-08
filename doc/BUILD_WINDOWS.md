# How to build on Windows

## How to build with MSVC on Windows

1. Something, something, open VisualStudio
2. Something, something, update VisualStudio
3. Install VulkanSDK and restart
4. Check for updates
5. Create a project solution and fix the mess

## How to build with GCC on Windows

1. Install MSYS2
2. Run "MSYS2 MSYS" from Start menu
3. Enter the following commands into MSYS2 console:
```
pacman -Syu
pacman -Su
pacman -S --needed base-devel mingw-w64-x86_64-toolchain
pacman -S cmake
```
4. Add to Path:
```
C:\msys64\usr\bin
C:\msys64\usr\local\bin
C:\msys64\mingw64\bin
C:\msys64\mingw32\bin
```
5. Clone the repository and run the following commands:
```
mkdir debug
cd debug
cmake --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_C_COMPILER:FILEPATH=C:\msys64\mingw64\bin\x86_64-w64-mingw32-gcc.exe -DCMAKE_CXX_COMPILER:FILEPATH=C:\msys64\mingw64\bin\x86_64-w64-mingw32-g++.exe .. -G "Unix Makefiles"
cmake --build .
```