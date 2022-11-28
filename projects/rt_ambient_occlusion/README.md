# RT ambient occlusion
An example project to show usage of hardware accelerated ray tracing with the VkCV framework

![Screenshot](../../screenshots/rt_ambient_occlusion.png)

## Details

The project utilizes multiple Vulkan extensions to make use of hardware accelerated ray tracing. 
Newer GPU architectures provide special hardware to allow the use of acceleration structures and 
ray tracing pipelines. This application uses those features to render an implementation of 
ambient occlusion in realtime.

## Extensions

Here is a list of the used extensions:

- [VK_KHR_buffer_device_address](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_buffer_device_address.html)
- [VK_KHR_acceleration_structure](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_acceleration_structure.html)
- [VK_KHR_ray_tracing_pipeline](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_ray_tracing_pipeline.html)
