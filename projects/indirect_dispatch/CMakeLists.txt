cmake_minimum_required(VERSION 3.16)
project(indirect_dispatch)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# adding source files to the project
add_project(indirect_dispatch src/main.cpp)

target_sources(indirect_dispatch PRIVATE
    src/App.hpp
    src/App.cpp

    src/AppConfig.hpp
    src/MotionBlurConfig.hpp
    
    src/AppSetup.hpp
    src/AppSetup.cpp
    
    src/MotionBlur.hpp
    src/MotionBlur.cpp
    
    src/MotionBlurSetup.hpp
    src/MotionBlurSetup.cpp)

# including headers of dependencies and the VkCV framework
target_include_directories(indirect_dispatch SYSTEM BEFORE PRIVATE
		${vkcv_include}
		${vkcv_includes}
		${vkcv_testing_include}
		${vkcv_camera_include}
		${vkcv_shader_compiler_include}
		${vkcv_gui_include}
		${vkcv_upscaling_include}
)

# linking with libraries from all dependencies and the VkCV framework
target_link_libraries(indirect_dispatch
		vkcv
		${vkcv_libraries}
		vkcv_asset_loader
		${vkcv_asset_loader_libraries}
		vkcv_testing
		vkcv_camera
		vkcv_shader_compiler
		vkcv_gui
		vkcv_upscaling
)