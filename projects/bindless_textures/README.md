# Bindless textures
An example project to show usage of bindless descriptor indexing with the VkCV framework

![Screenshot](../../screenshots/bindless_textures.png)

## Details

The project utilizes a Vulkan extension to access an array of descriptors with arbitrary size. The 
size does not need to be known during shader compilation and can be adjusted during runtime when
creating the graphics pipeline.

## Extensions

Here is a list of the used extensions:

 - [VK_EXT_descriptor_indexing](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_EXT_descriptor_indexing.html)
