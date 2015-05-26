#!/bin/bash

# Copyright (c) 2015, Intel Corporation
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation and/or
# other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# This script produces the DebugAgent build files using cmake
# Build files are created into build/<platform name>

# This script intends to work on Linux and on Windows using MinGW

# This script takes one parameter: the platform name.
# These platforms are currently supported:
# - Linux64
# - Win64

# Checking specified platform
platform=$1

if [ "$platform" = "" ]
then
    echo "cmake.sh <platform> [--clean]"
    echo "  <platform> : Win64, Linux"
    echo "  --clean    : clean cmake files"
    exit 1
fi

# Setting generator and build function according to the platform
if [ "$platform" = "Win64" ]
then
    cmake_generator="Visual Studio 12 2013 Win64"
elif [ "$platform" = "Linux" ]
then
    cmake_generator="Unix Makefiles"
else
    echo Unknown platform: "$platform"
    exit 1
fi

# Option handling
param=$2
if [ "$param" = "--clean" ]
then
    clean=true
elif [ "$param" != "" ]
then
    echo Unknown option: "$param"
    exit 1
fi

# Setting directories
root_dir=$(cd "$(dirname "$0")" && pwd)
build_dir=$root_dir/build/$platform
install_dir=$root_dir/install/$platform
poco_dir=$root_dir/external/poco/install/$platform/lib/cmake/Poco

echo Directories:
echo   root_dir=$root_dir
echo   build_dir=$build_dir
echo   install_dir=$install_dir
echo   poco_dir=$poco_dir
echo

# Cleaning build directory if required
if [ $clean ] && [ -d $build_dir ]
then
  echo Removing directory $build_dir
  rm -rf $build_dir
fi

# Creating the directory if it does not exist
if [ ! -d $build_dir ]
then
    mkdir -p $build_dir
fi

# Running cmake
cd $build_dir
echo Running cmake for platform $platform...
cmake $root_dir -G "$cmake_generator" -DCMAKE_INSTALL_PREFIX=$install_dir -DPoco_DIR=$poco_dir

# Coming back to initial directory
cd $root_dir

echo Done.



