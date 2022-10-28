# How to build on macOS

## How to build with Clang on macOS

1. Install Homebrew
2. Install required tools to compile:
```
brew install cmake llvm libomp
```
3. Download and install latest [vulkansdk-macos-*.dmg](https://vulkan.lunarg.com/doc/sdk/1.2.189.0/mac/getting_started.html) to ~/VulkanSDK
4. Clone the repository and run the following commands:
```
mkdir debug
cd debug
export LDFLAGS="-L/usr/local/opt/llvm/lib"
export CPPFLAGS="-I/usr/local/opt/llvm/include"
cmake -DCMAKE_C_COMPILER="/usr/local/opt/llvm/bin/clang" -DCMAKE_CXX_COMPILER="/usr/local/opt/llvm/bin/clang++" -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```