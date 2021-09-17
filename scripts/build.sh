#!/bin/sh

# Navigate to the main directory of the cloned repository
BASEDIR=$(dirname "$0")
cd $BASEDIR
cd ..

# Setup git lfs and the submodules
git lfs install
git submodule init
git submodule update

# Setup build directory
mkdir build
cd build

# Build process
cmake ..
make