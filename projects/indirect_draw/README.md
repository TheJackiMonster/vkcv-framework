# Indirect draw
An example project to show usage of indirect draw calls

![Screenshot](../../screenshots/indirect_draw.png)

## Details

The project utilizes multiple Vulkan extensions to access to make indirect draw calls which can be
modified on the GPU from within compute shaders. This will be used to cull the rendered scene on
the GPU instead of (potentially) more expensive frustum culling on the CPU.

## Extensions

Here is a list of the used extensions:

 - [VK_KHR_shader_draw_parameters](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_shader_draw_parameters.html)
 - [VK_EXT_descriptor_indexing](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_EXT_descriptor_indexing.html)
