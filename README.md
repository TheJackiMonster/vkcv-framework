# VkCV Framework
 A Vulkan framework for computer visualistics simplifying building applications

![Vulkan-Chan](https://gitlab.uni-koblenz.de/uploads/-/system/project/avatar/3712/VulkanChan.jpg)

## Repository

Git LFS is used for bigger resource files like meshes and textures. So you need to install Git LFS and use `git lfs install` after cloning.

More information about Git LFS [here](https://git-lfs.github.com/).

## Build

Git submodules are used for libraries. 
To download the submodules either clone using `git clone --recurse-submodules` or after `git clone` use `git submodule init` and `git submodule update`.

Detailed build process:
 - [How to build on Windows](doc/BUILD_WINDOWS.md)
 - [How to build on macOS](doc/BUILD_MACOS.md)
 - [How to build on Linux](doc/BUILD_LINUX.md)

### Dependencies (required):

Most dependencies will be used via submodules but for example Vulkan needs to be installed correctly depending on your platform. So please setup your environment properly.

| Name of dependency | Used as submodule |
|-----------------------------------|---|
| [Vulkan](https://www.vulkan.org/) | ❌ |
| [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers) | ✅ |
| [Vulkan-Hpp](https://github.com/KhronosGroup/Vulkan-Hpp) | ✅ |
| [GLFW](https://www.glfw.org/) | ✅ |
| [SPIRV-CROSS](https://github.com/KhronosGroup/SPIRV-Cross) | ✅ |
| [VulkanMemoryAllocator-Hpp](https://github.com/malte-v/VulkanMemoryAllocator-Hpp) | ✅ |

### Modules (optional):

The following modules will be provided in this repository and they will automatically be builded together with the framework if used. You can configure/adjust the build using CMake if necessary.

 - [Asset-Loader](modules/asset_loader/README.md)
 - [Camera](modules/asset_loader/README.md)
 - [GUI](modules/gui/README.md)
 - [Effects](modules/effects/README.md)
 - [Material](modules/material/README.md)
 - [Meshlet](modules/meshlet/README.md)
 - [Scene](modules/scene/README.md)
 - [Shader-Compiler](modules/shader_compiler/README.md)
 - [Upscaling](modules/upscaling/README.md)

## Development

See this guide to setup your IDE for most forward development.
 - [How to setup your IDE](doc/SETUP_IDE.md)

## Documentation

A pre-built documentation can be found here:  
https://userpages.uni-koblenz.de/~vkcv/doc/

But it is recommended to build the documentation with Doxygen locally to get the most recent changes. There is also an optional CMake target to build the documentation via Doxygen.
