# Voxelization
An example project to show a volumetric global illumination technique with the VkCV framework

![Screenshot](../../screenshots/voxelization.png)

## Details

The project utilizes multiple Vulkan extensions (some of them optionally) to implement multiple 
graphics effects in an advanced visualization of a whole scene. Together this brings visual 
features like shadow maps, indirect lighting via discrete voxels and multiple post-processing 
effects like bloom and lens-flares to the screen. To make all of this available in realtime even 
on low powered hardware, it is possible to utilize multiple different upscaling methods.

## Extensions

Here is a list of the used extensions:

- [VK_EXT_descriptor_indexing](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_EXT_descriptor_indexing.html)
- [VK_KHR_shader_subgroup_extended_types](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_shader_subgroup_extended_types.html)
- [VK_KHR_shader_float16_int8](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_shader_float16_int8.html)
- [VK_KHR_16bit_storage](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_16bit_storage.html)
