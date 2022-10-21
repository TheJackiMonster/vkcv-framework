# RTX ambient occlusion
An example project to show usage of hardware accelerated ray tracing with the VkCV framework

![Screenshot](../../screenshots/rtx_ambient_occlusion.png)

## Details

The project utilizes multiple Vulkan extensions to make use of hardware accelerated ray tracing. 
Newer GPU architectures provide special hardware to allow the use of acceleration structures and 
ray tracing pipelines. This application uses those features to render an implementation of 
ambient occlusion in realtime.

## Extensions

Here is a list of the used extensions:

- [VK_KHR_get_physical_device_properties2](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_get_physical_device_properties2.html)
- [VK_KHR_maintenance3](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_maintenance3.html)
- [VK_KHR_deferred_host_operations](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_deferred_host_operations.html)
- [VK_KHR_spirv_1_4](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_spirv_1_4.html)
- [VK_KHR_pipeline_library](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_pipeline_library.html)
- [VK_EXT_descriptor_indexing](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_EXT_descriptor_indexing.html)
- [VK_KHR_buffer_device_address](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_buffer_device_address.html)
- [VK_KHR_acceleration_structure](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_acceleration_structure.html)
- [VK_KHR_ray_tracing_pipeline](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_ray_tracing_pipeline.html)
