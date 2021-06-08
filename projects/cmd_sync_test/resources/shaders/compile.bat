%VULKAN_SDK%\Bin32\glslc.exe shader.vert -o vert.spv
%VULKAN_SDK%\Bin32\glslc.exe shader.frag -o frag.spv
%VULKAN_SDK%\Bin32\glslc.exe shadow.vert -o shadow_vert.spv
%VULKAN_SDK%\Bin32\glslc.exe shadow.frag -o shadow_frag.spv
%VULKAN_SDK%\Bin32\glslc.exe voxelization.vert -o voxelization_vert.spv
%VULKAN_SDK%\Bin32\glslc.exe voxelization.frag -o voxelization_frag.spv
pause