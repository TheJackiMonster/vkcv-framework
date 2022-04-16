#!/bin/sh
# Check if release or debug build
CMAKE_BUILD_DIR="build"
CMAKE_FLAGS=""
if [ "$1" = "--debug" ]; then
	CMAKE_BUILD_DIR="cmake-build-debug"
	CMAKE_FLAGS="-DCMAKE_BUILD_TYPE=Debug"
elif [ "$1" = "--release" ]; then
	CMAKE_BUILD_DIR="cmake-build-release"
	CMAKE_FLAGS="-DCMAKE_BUILD_TYPE=Release"
fi

# Navigate to the main directory of the cloned repository
cd "$(dirname "$0")" || exit
cd ..

# Setup git lfs and the submodules
git lfs install
git submodule init
git submodule update

# Setup build directory
mkdir $CMAKE_BUILD_DIR
cd $CMAKE_BUILD_DIR || exit
BUILD_THREADS=$(($(nproc --all) - 1))

if [ $BUILD_THREADS -lt 1 ]; then
	BUILD_THREADS=1
fi

# Build process
cmake $CMAKE_FLAGS ..
make -j $BUILD_THREADS "$@"