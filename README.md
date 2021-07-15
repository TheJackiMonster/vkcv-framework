# VkCV Framework
 A Vulkan framework for computer visualistics simplifying building applications

![Vulkan-Chan](https://gitlab.uni-koblenz.de/uploads/-/system/project/avatar/3712/VulkanChan.jpg)

## Repository

Git LFS is used for bigger resource files like meshes and textures. So you need to install Git LFS and use `git lfs install` after cloning.

More information about Git LFS [here](https://git-lfs.github.com/).

## Build

 [![pipeline status](https://gitlab.uni-koblenz.de/vulkan2021/vkcv-framework/badges/develop/pipeline.svg)](https://gitlab.uni-koblenz.de/vulkan2021/vkcv-framework/-/commits/develop)

Git submodules are used for libraries. 
To download the submodules either clone using `git clone --recurse-submodules` or after `git clone` use `git submodule init` and `git submodule update`.

### Dependencies (required):

Most dependencies will be used via submodules but for example Vulkan needs to be installed correctly depending on your platform. So please setup your environment properly.

| Name of dependency | Used as submodule |
|-----------------------------------|---|
| [Vulkan](https://www.vulkan.org/) | ❌ |
| [GLFW](https://www.glfw.org/) | ✅ |
| [SPIRV-CROSS](https://github.com/KhronosGroup/SPIRV-Cross) | ✅ |
| [VulkanMemoryAllocator-Hpp](https://github.com/malte-v/VulkanMemoryAllocator-Hpp) | ✅ |

### Modules (optional):

The following modules will be provided in this repository and they will automatically be builded together with the framework if used. You can configure/adjust the build using CMake if necessary.

 - [Asset-Loader](modules/asset_loader/README.md)
 - [Camera](modules/asset_loader/README.md)
 - [GUI](modules/gui/README.md)
 - [Material](modules/material/README.md)
 - [Scene](modules/scene/README.md)
 - [Shader-Compiler](modules/scene/README.md)

## Documentation

The documentation for the develop-branch can be found here:  
https://vkcv.de/develop/  

The documentation concerning the respective merge request is listed here:  
https://vkcv.de/branch/  

It is automatically generated and uploaded using the CI pipeline.
