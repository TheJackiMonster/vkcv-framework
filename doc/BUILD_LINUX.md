# How to build on Linux

## How to build with GCC on Archlinux by the way

1. Install required and useful tools to develop:
```
sudo pacman -S cmake gcc vulkan-icd-loader vulkan-headers vulkan-validation-layers vulkan-tools
```
2. Install the matching Vulkan drivers for your system:
```
# For discrete Nvidia GPUs:
sudo pacman -S nvidia

# For integrated or discrete Radeon GPUs:
sudo pacman -S vulkan-radeon

# For integrated or discrete Intel GPUs:
sudo pacman -S vulkan-intel
```
3. Clone the repository and run the following commands:
```
mkdir debug
cd debug
cmake -DCMAKE_C_COMPILER="/usr/bin/gcc" -DCMAKE_CXX_COMPILER="/usr/bin/g++" -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

## How to build with Clang on Archlinux by the way

1. Install required and useful tools to develop:
```
sudo pacman -S cmake clang vulkan-icd-loader vulkan-headers vulkan-validation-layers vulkan-tools
```
2. Install the matching Vulkan drivers for your system:
```
# For discrete Nvidia GPUs:
sudo pacman -S nvidia

# For integrated or discrete Radeon GPUs:
sudo pacman -S vulkan-radeon

# For integrated or discrete Intel GPUs:
sudo pacman -S vulkan-intel
```
3. Clone the repository and run the following commands:
```
mkdir debug
cd debug
cmake -DCMAKE_C_COMPILER="/usr/bin/clang" -DCMAKE_CXX_COMPILER="/usr/bin/clang++" -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```
