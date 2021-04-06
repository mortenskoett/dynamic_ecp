#! /usr/bin/env bash

# Script to be copied into ann-benchmarks dir to automatically run and move around benchmarks
# for absolute convenience. Uncomment dataset to be run.

NAME="ecp_run_local"
# DATASET="../eCP/datasets/random-xs-20-euclidean"
DATASET="glove-25-angular"

echo "${NAME}: Will run benchmarks on eCP. Important to run 'ecp_install.sh' and set correct DATASET."
read -p "${NAME}: Press enter to continue"

# Go to ann-benchmarks dir
cd ../../ann-benchmarks

echo "${NAME}: Setup python 3.6 env inside ann-benchmarks"
python3.6 -m venv env
source env/bin/activate

echo "${NAME}: Running tests"
python run.py --local --algorithm eCP --dataset ${DATASET}

echo "${NAME}: Creating plots and website"
python plot.py --dataset ${DATASET}

mkdir -p website
python create_website.py --outputdir website

# echo "Cleaning up tests"

echo "${NAME}: Done."
