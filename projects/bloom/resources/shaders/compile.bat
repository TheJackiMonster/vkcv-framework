%VULKAN_SDK%\Bin32\glslc.exe shader.vert -o vert.spv
%VULKAN_SDK%\Bin32\glslc.exe shader.frag -o frag.spv

%VULKAN_SDK%\Bin32\glslc.exe shadow.vert -o shadow_vert.spv
%VULKAN_SDK%\Bin32\glslc.exe shadow.frag -o shadow_frag.spv

%VULKAN_SDK%\Bin32\glslc.exe quad.vert -o quad_vert.spv
%VULKAN_SDK%\Bin32\glslc.exe quad.frag -o quad_frag.spv

%VULKAN_SDK%\Bin32\glslc.exe blur.comp   -o blur_comp.spv
pause