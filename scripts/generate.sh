#!/bin/sh
CMAKE_PROJECT_DIR="$(pwd)"
CMAKE_PROJECT_NAME="$(basename "$CMAKE_PROJECT_DIR")"

# Navigate to the main directory of the cloned repository
cd "$(dirname "$0")" || exit
cd ..

CMAKE_FRAMEWORK_DIR="$(realpath -s --relative-to="$CMAKE_PROJECT_DIR" "$(pwd)")"

# Navigate back to the project directory
cd "$CMAKE_PROJECT_DIR" || exit

test -f "CMakeLists.txt" && echo "WARNING: CMakeLists.txt exists already! Project generation stops!" && exit
test -f "src/main.cpp" && echo "WARNING: src/main.cpp exists already! Project generation stops!" && exit

generate_cmake_lists() {
  echo "cmake_minimum_required(VERSION 3.16)"
  echo "project($CMAKE_PROJECT_NAME)"
  echo
  echo "set(CMAKE_CXX_STANDARD 20)"
  echo "set(CMAKE_CXX_STANDARD_REQUIRED ON)"
  echo
  echo "set(BUILD_MODULES ON CACHE INTERNAL \"\")"
  echo "set(BUILD_PROJECTS OFF CACHE INTERNAL \"\")"
  echo "set(BUILD_DOXYGEN_DOCS OFF CACHE INTERNAL \"\")"
  echo "set(BUILD_SHARED OFF CACHE INTERNAL \"\")"
  echo "add_subdirectory($CMAKE_FRAMEWORK_DIR)"
  echo
  echo "add_executable($CMAKE_PROJECT_NAME src/main.cpp)"
  echo
  echo "target_include_directories($CMAKE_PROJECT_NAME SYSTEM BEFORE PRIVATE \${vkcv_includes})"
  echo "target_link_libraries($CMAKE_PROJECT_NAME \${vkcv_libraries})"
}

generate_main_cpp() {
  echo "#include <vkcv/Core.hpp>"
  echo
  echo "int main(int argc, const char** argv) {"
  echo "  vkcv::Core core = vkcv::Core::create("
  echo "    \"$CMAKE_PROJECT_NAME\","
  echo "    VK_MAKE_VERSION(0, 0, 1),"
  echo "    { vk::QueueFlagBits::eTransfer,vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute },"
  echo "    { VK_KHR_SWAPCHAIN_EXTENSION_NAME }"
  echo "  );"
  echo "  "
  echo "  vkcv::WindowHandle windowHandle = core.createWindow(\"$CMAKE_PROJECT_NAME\", 800, 600, true);"
  echo "  "
  echo "  while (vkcv::Window::hasOpenWindow()) {"
  echo "    vkcv::Window::pollEvents();"
  echo "  }"
  echo "}"
}

generate_cmake_lists > "CMakeLists.txt"

mkdir -p "src"
generate_main_cpp > "src/main.cpp"
