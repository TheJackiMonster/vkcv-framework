%VULKAN_SDK%\Bin32\glslc.exe shader.vert -o vert.spv
%VULKAN_SDK%\Bin32\glslc.exe shader.frag -o frag.spv

%VULKAN_SDK%\Bin32\glslc.exe shadow.vert -o shadow_vert.spv
%VULKAN_SDK%\Bin32\glslc.exe shadow.frag -o shadow_frag.spv

%VULKAN_SDK%\Bin32\glslc.exe voxelization.vert -o voxelization_vert.spv
%VULKAN_SDK%\Bin32\glslc.exe voxelization.geom -o voxelization_geom.spv
%VULKAN_SDK%\Bin32\glslc.exe voxelization.frag -o voxelization_frag.spv

%VULKAN_SDK%\Bin32\glslc.exe voxelVisualisation.vert -o voxelVisualisation_vert.spv
%VULKAN_SDK%\Bin32\glslc.exe voxelVisualisation.geom -o voxelVisualisation_geom.spv
%VULKAN_SDK%\Bin32\glslc.exe voxelVisualisation.frag -o voxelVisualisation_frag.spv
pause