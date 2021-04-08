#! /usr/bin/env bash

# Script to be copied into ann-benchmarks dir to automatically run and move around benchmarks
# for absolute convenience. Uncomment dataset to be run.

NAME="ecp_run_local"
DATASET="random-xs-20-euclidean"
# DATASET="glove-25-angular"

echo "${NAME}: Will run benchmarks on eCP. First run 'ecp_install.sh'. Dataset: $DATASET."
read -p "${NAME}: Press enter to continue"

echo "${NAME}: Copy datasets"
yes | cp -rv ../datasets/${DATASET}.hdf5 ../../ann-benchmarks/data/${DATASET}.hdf5

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
python create_website.py --outputdir website --scatter

# echo "Cleaning up tests"

echo "${NAME}: Done."
