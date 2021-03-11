#!/usr/bin/env bash

set -e
set -o pipefail

# Script to construct a python wrapper around the C++ code using SWIG.
# This code is called inside docker image when built alongside other 
# algorithms in the ann-benchmarks repo.
# Currently .so and .py can be found both in: '<eCP_root>/eCP/build/swig/'

# Dependencies: cmake

NAME="gen_wrapper"
# DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

echo "${NAME}: Setting up python 3.6 env..."
cd ..		# root ecp dir
python3.6 -m venv env
source env/bin/activate

echo "${NAME}: Compiling project incl SWIG generated shared library..."
cd eCP/
[ ! -d build ] \
&& echo "${NAME}: Creating build dir..." \
&& mkdir build

echo "${NAME}: Configuring Cmake..."
cmake -S . -B build

echo "${NAME}: Building Cmake..."
cmake --configure build
cmake --build build

echo "${NAME}: Wrapper generation successful."