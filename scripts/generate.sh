#!/bin/sh
if [ $# -gt 0 ]; then
  CMAKE_PROJECT_DIR="$(realpath "$1")"
  mkdir -p $CMAKE_PROJECT_DIR
else
  CMAKE_PROJECT_DIR="$(pwd)"
fi

CMAKE_PROJECT_NAME="$(basename "$CMAKE_PROJECT_DIR")"

# Navigate to the main directory of the cloned repository
cd "$(dirname "$0")" || exit
cd ..

CMAKE_FRAMEWORK_DIR="$(realpath -s --relative-to="$CMAKE_PROJECT_DIR" "$(pwd)")"

if [ "$CMAKE_FRAMEWORK_DIR" == "../.." ]; then
  IS_INTERNAL_PROJECT=true
else
  IS_INTERNAL_PROJECT=false
fi

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

  if test "$IS_INTERNAL_PROJECT" = false; then
    echo "set(BUILD_MODULES ON CACHE INTERNAL \"\")"
    echo "set(BUILD_PROJECTS OFF CACHE INTERNAL \"\")"
    echo "set(BUILD_DOXYGEN_DOCS OFF CACHE INTERNAL \"\")"
    echo "set(BUILD_SHARED OFF CACHE INTERNAL \"\")"
    echo "add_subdirectory($CMAKE_FRAMEWORK_DIR)"
    echo
  fi
  
  echo "add_executable($CMAKE_PROJECT_NAME src/main.cpp)"
  echo

  if test "$IS_INTERNAL_PROJECT" = true; then
    echo "target_include_directories($CMAKE_PROJECT_NAME SYSTEM BEFORE PRIVATE \${vkcv_include} \${vkcv_includes})"
    echo "target_link_libraries($CMAKE_PROJECT_NAME vkcv \${vkcv_libraries})"
  else
    echo "target_include_directories($CMAKE_PROJECT_NAME SYSTEM BEFORE PRIVATE \${vkcv_includes})"
    echo "target_link_libraries($CMAKE_PROJECT_NAME \${vkcv_libraries})"
  fi
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
  echo "  vkcv::WindowHandle windowHandle = core.createWindow("
  echo "    \"$CMAKE_PROJECT_NAME\","
  echo "    800,"
  echo "    600,"
  echo "    true"
  echo "  );"
  echo "  "
  echo "  while (vkcv::Window::hasOpenWindow()) {"
  echo "    vkcv::Window::pollEvents();"
  echo "  }"
  echo "}"
}

generate_cmake_lists > "CMakeLists.txt"

mkdir -p "src"
generate_main_cpp > "src/main.cpp"
