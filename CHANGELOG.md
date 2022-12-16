# Changelog

## [0.2.0](https://gitlab.uni-koblenz.de/vulkan2021/vkcv-framework/tree/0.2.0) (2021-12-16)

** New modules:**

 - [Algorithm](modules/algorithm/README.md): A VkCV module to use optimized compute algorithms accelerating projects
 - [Effects](modules/effects/README.md): A VkCV module to use pre-built post-processing effects in your projects
 - [Geometry](modules/geometry/README.md): A VkCV module to generate basic geometry for rendering without loading a mesh from files
 - [Tone-Mapping](modules/tone_mapping/README.md): A VkCV module providing multiple pre-defined options for tone mapping

** New features:**

 - Compile shaders from memory with defined headers
 - Hardware accelerated ray tracing pipelines
 - Dynamic geometry generation
 - Interpolation
 - Tesselation
 - Cube maps
 - Temporal upscaling
 - Automated camera paths

** Improvements **

 - Much more documentation
 - Fully refactored API to ease usage
 - More consistency
 - Less duplicate code

## [0.1.0](https://gitlab.uni-koblenz.de/vulkan2021/vkcv-framework/tree/0.1.0) (2021-12-07)

** Platform support:**

 - Linux support (GCC and CLang)
 - MacOS support (Apple CLang)
 - Windows support (MSVC and MinGW-GCC experimentally)

** New modules:**

 - [Asset-Loader](modules/asset_loader/README.md): A VkCV module to load basic assets like models, materials and images
 - [Camera](modules/asset_loader/README.md): A VkCV module to manage cameras and their handle view and projection matrices
 - [GUI](modules/gui/README.md): A VkCV module to integrate GUI rendering to your application as additional pass
 - [Material](modules/material/README.md): A VkCV module to abstract typical kinds of materials for rendering
 - [Meshlet](modules/meshlet/README.md): A VkCV module to divide vertex data of a mesh into meshlets
 - [Scene](modules/scene/README.md): A VkCV module to load and manage a scene, simplify its rendering and potentially optimize it
 - [Shader-Compiler](modules/shader_compiler/README.md): A VkCV module to compile shaders at runtime
 - [Upscaling](modules/upscaling/README.md): A VkCV module to upscale images in realtime

** New features:**

 - Resizable windows
 - Multiple windows and multiple swapchains (window management)
 - Dynamically requesting Vulkan features and extensions
 - Shader reflection and runtime shader compilation (various shader stages)
 - Realtime ray tracing
 - Mesh shaders
 - Indirect dispatch
 - Compute pipelines and compute shaders
 - Multiple queues and graphic pipelines
 - Bindless textures
 - ImGUI support
 - Mipmapping
 - Logging
 - Command buffer synchronization
 - Doxygen source code documentation
 - Buffer, sampler and image management
 - Camera management with gamepad support
 - Input event synchronization
 - Resource management with handles

