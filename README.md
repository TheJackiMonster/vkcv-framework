# VkCV Framework
 A Vulkan framework for computer visualistics simplifying building applications

[![linux-build-on-github](https://github.com/thejackimonster/vkcv-framework/workflows/Linux%20Build/badge.svg)](https://github.com/TheJackiMonster/vkcv-framework/actions/workflows/linux.yml)
[![windows-build-on-github](https://github.com/thejackimonster/vkcv-framework/workflows/Windows%20Build/badge.svg)](https://github.com/TheJackiMonster/vkcv-framework/actions/workflows/windows.yml)
[![macos-build-on-github](https://github.com/thejackimonster/vkcv-framework/workflows/macOS%20Build/badge.svg)](https://github.com/TheJackiMonster/vkcv-framework/actions/workflows/macos.yml)

![Vulkan-Chan](https://gitlab.uni-koblenz.de/uploads/-/system/project/avatar/3712/VulkanChan.jpg)

## Repository

Git LFS is used for bigger resource files like meshes and textures. So you need to install Git LFS 
and use `git lfs install` after cloning.

More information about Git LFS [here](https://git-lfs.github.com/).

## Build

Git submodules are used for libraries. To download the submodules either clone using 
`git clone --recurse-submodules` or after `git clone` use `git submodule init` and 
`git submodule update`.

Detailed build process:
 - [How to build on Windows](doc/BUILD_WINDOWS.md)
 - [How to build on macOS](doc/BUILD_MACOS.md)
 - [How to build on Linux](doc/BUILD_LINUX.md)

### Dependencies (required):

Most dependencies are used via submodules but for example Vulkan needs to be installed correctly 
depending on your platform. So please setup your environment properly.

| Name of dependency                                                                | Used as submodule |
|-----------------------------------------------------------------------------------|---|
| [Vulkan](https://www.vulkan.org/)                                                 | ❌ |
| [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers)                  | ✅ |
| [Vulkan-Hpp](https://github.com/KhronosGroup/Vulkan-Hpp)                          | ✅ |
| [GLFW](https://www.glfw.org/)                                                     | ✅ |
| [SPIRV-CROSS](https://github.com/KhronosGroup/SPIRV-Cross)                        | ✅ |
| [VulkanMemoryAllocator-Hpp](https://github.com/malte-v/VulkanMemoryAllocator-Hpp) | ✅ |

### Modules (optional):

The following modules are provided in this repository and they build automatically together with 
the framework if used. You can configure/adjust the build using CMake if necessary.

 - [Algorithm](modules/algorithm/README.md)
 - [Asset-Loader](modules/asset_loader/README.md)
 - [Camera](modules/asset_loader/README.md)
 - [Effects](modules/effects/README.md)
 - [Geometry](modules/geometry/README.md)
 - [GUI](modules/gui/README.md)
 - [Material](modules/material/README.md)
 - [Meshlet](modules/meshlet/README.md)
 - [Scene](modules/scene/README.md)
 - [Shader-Compiler](modules/shader_compiler/README.md)
 - [Tone-Mapping](modules/tone_mapping/README.md)
 - [Upscaling](modules/upscaling/README.md)

### Projects (optional):

The following projects are provided in this repository and can be build with their own CMake 
targets:

 - [bindless_textures](projects/bindless_textures/README.md)
 - [fire_works](projects/fire_works/README.md)
 - [first_mesh](projects/first_mesh/README.md)
 - [first_scene](projects/first_scene/README.md)
 - [first_triangle](projects/first_triangle/README.md)
 - [head_demo](projects/head_demo/README.md)
 - [indirect_dispatch](projects/indirect_dispatch/README.md)
 - [indirect_draw](projects/indirect_draw/README.md)
 - [mesh_shader](projects/mesh_shader/README.md)
 - [mpm](projects/mpm/README.md)
 - [particle_simulation](projects/particle_simulation/README.md)
 - [path_tracer](projects/path_tracer/README.md)
 - [ray_tracer](projects/ray_tracer/README.md)
 - [rt_ambient_occlusion](projects/rt_ambient_occlusion/README.md)
 - [sph](projects/sph/README.md)
 - [voxelization](projects/voxelization/README.md)

## Development

See this guide to setup your IDE for most forward development.
 - [How to setup your IDE](doc/SETUP_IDE.md)

## Documentation

A pre-built documentation can be found here:  
https://userpages.uni-koblenz.de/~vkcv/doc/

But it is recommended to build the documentation with Doxygen locally to get the most recent 
changes. There is also an optional CMake target to build the documentation via Doxygen.
