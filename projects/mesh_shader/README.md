# Mesh shader
An example project to show usage of mesh shaders with the VkCV framework

![Screenshot](../../screenshots/mesh_shader.png)

## Details

The project utilizes a Vulkan extension to use hardware accelerated mesh shaders. Those mesh 
shaders can replace the usual vertex-, tessellation- and geometry-shader stages in the graphics 
pipeline. They also act more similar to compute shaders.

The application uses those mesh shaders to cull a rendered mesh into individual groups of triangles 
which are called meshlets in this context.

The project currently uses the Nvidia GPU exclusive extension for mesh shading but it could be 
adjusted to use the cross compatible extension 
"[VK_EXT_mesh_shader](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_EXT_mesh_shader.html)" 
instead.

## Extensions

Here is a list of the used extensions:

- [VK_NV_mesh_shader](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_NV_mesh_shader.html)
