# How to build on Linux

## How to build on Ubuntu

1. Install the required and useful packages to develop:
```
sudo apt install cmake make gcc g++ libvulkan-dev vulkan-validationlayers-dev vulkan-tools
```
2. Install the Vulkan drivers for your system:
```
# For most GPUs (except Nvidia):
sudo apt install mesa-vulkan-drivers
```
3. Clone the repository and run the following commands:
```
mkdir debug
cd debug
cmake -DCMAKE_C_COMPILER="/usr/bin/gcc" -DCMAKE_CXX_COMPILER="/usr/bin/g++" -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

## How to build on Archlinux by the way

1. Install required and useful tools to develop:
```
sudo pacman -S cmake make gcc vulkan-icd-loader vulkan-headers vulkan-validation-layers vulkan-tools
```
2. Install the matching Vulkan drivers for your system:
```
# For discrete Nvidia GPUs:
sudo pacman -S nvidia

# For integrated or discrete (AMD) Radeon GPUs:
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

## How to build with Clang on Linux

Just follow the steps of "How to build on YOUR DISTRO" but replace `gcc` with `clang` and `g++` with `clang++`.
This should work actually but maybe the names of the packages are a little different. So look at your official 
repository for the packages and their names if it doesn't work.
