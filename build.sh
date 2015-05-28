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

# This script allows to clean, generate build system, compile and run unit tests
# of the Debug Agent.
# Build files are created into build/<platform name>
# Install files are created into install/<platform name>

# This script intends to work on Linux and on Windows using MinGW

# This script takes one parameter: the platform name.
# These platforms are currently supported:
# - Linux
# - Win64

# Produce install binaries on windows
function build_win64
{
    cmake --build . --config release --target install
}

# Produce install binaries on linux
function build_linux
{
    make && make install
}

# Checking parameters
platform=$1
shift
actions=$@

if [ "$platform" = "" ] || [ "$actions" = "" ]
then
    echo "build.sh <platform> [<action>]"
    echo "  <platform> : Win64, Linux"
    echo "  <action>   : clean, cmake, install, test, all"
    echo
    echo "Example:"
    echo "  build.sh Linux clean cmake"
    exit 1
fi

# Finding actions
for action in $actions
do
    case "$action" in
        clean)
            action_clean=true
            ;;
        cmake)
            action_cmake=true
            ;;
        install)
            action_install=true
            ;;
        test)
            action_test=true
            ;;
        all)
            action_clean=true
            action_cmake=true
            action_install=true
            action_test=true
            ;;
        *)
            echo "Invalid action: $action"
            exit 1;
            ;;
    esac
done

# Setting generator and build function according to the platform
if [ "$platform" = "Win64" ]
then
    cmake_generator="Visual Studio 12 2013 Win64"
    build_function="build_win64"
elif [ "$platform" = "Linux" ]
then
    cmake_generator="Unix Makefiles"
    build_function="build_linux"
else
    echo Unknown platform: "$platform"
    exit 1
fi

# Setting directories
root_dir=$(cd "$(dirname "$0")" && pwd)
build_dir=$root_dir/build/$platform
install_dir=$root_dir/install/$platform
test_dir=$install_dir/tests
poco_dir=$root_dir/external/poco/install/$platform/lib/cmake/Poco

echo Directories:
echo   root_dir=$root_dir
echo   build_dir=$build_dir
echo   install_dir=$install_dir
echo   poco_dir=$poco_dir
echo

#cleaning
if [ $action_clean ]
then
    echo "-----------"
    echo "Cleaning..."
    echo "-----------"
    # Cleaning build directory
    if [ -d $build_dir ]
    then
      echo Removing directory $build_dir
      rm -rf $build_dir
    fi

    # Cleaning install directory
    if [ -d $install_dir ]
    then
      echo Removing directory $install_dir
      rm -rf $install_dir
    fi
fi

# Running cmake
if [ $action_cmake ]
then
    echo
    echo "----------------"
    echo "Running cmake..."
    echo "----------------"

    # Creating the build directory if it does not exist
    if [ ! -d $build_dir ]
    then
        mkdir -p $build_dir
    fi

    cd $build_dir
    cmake $root_dir -G "$cmake_generator" -DCMAKE_INSTALL_PREFIX=$install_dir -DPoco_DIR=$poco_dir
    cmake_result=$?

    # Coming back to initial directory
    cd $root_dir

    if [ $cmake_result != 0 ]
    then
        echo cmake failed.
        exit 1
    fi
fi

# Compiling and installing
if [ $action_install ]
then
    echo
    echo "---------------------------"
    echo "Compiling and installing..."
    echo "---------------------------"

    cd $build_dir
    eval $build_function
    build_result=$?
    cd $root_dir

    if [ $build_result != 0 ]
    then
        echo Build failed.
        exit 1
    fi
fi

# testing
if [ $action_test ]
then
    echo
    echo "----------------"
    echo "Running tests..."
    echo "----------------"
    test_count=0
    if [ -d $test_dir ]
    then
        for test_file in $test_dir/*UnitTest*
        do
            echo "Running $test_file"
            eval $test_file
            if [ $? != 0 ]
            then
                echo "Unit test failed: $test_file"
                exit 1;
            fi
            test_count=$((test_count+1))
        done
    fi

    if [ $test_count == 0 ]
    then
        echo "No test found."
        exit 1;
    fi

    echo "Run test count: $test_count"
fi

echo Done.



