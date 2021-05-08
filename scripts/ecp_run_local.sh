#! /usr/bin/env bash

# Script to be copied into ann-benchmarks dir to automatically run and move around benchmarks
# for absolute convenience. Uncomment dataset to be run.

NAME="ecp_run_local"

#DATASET="random-xs-20-euclidean"
#DATASET="glove-25-angular"
DATASET="sift-128-euclidean"
# DATASET="fashion-mnist-784-euclidean"

echo "${NAME}: Will run benchmarks on eCP-Original. First run 'ecp_install.sh'. Dataset: $DATASET."
read -p "${NAME}: Press enter to continue"

# Copy saved dataset if it exists
FILE="../datasets/${DATASET}.hdf5"
if [[ -f ${FILE} ]]; then
  echo "${NAME}: Copy datasets"
  yes | cp -rv ${FILE} ../../ann-benchmarks/data/${DATASET}.hdf5
fi

# Go to ann-benchmarks dir
cd ../../ann-benchmarks

echo "${NAME}: Setup python 3.6 env inside ann-benchmarks"
python3.6 -m venv env
source env/bin/activate

echo "${NAME}: Running tests"
python run.py --local --algorithm eCP-Original --dataset ${DATASET}

echo "${NAME}: Creating plots and website"
python plot.py --dataset ${DATASET}

mkdir -p website
python create_website.py --outputdir website --scatter

# echo "Cleaning up tests"

echo "${NAME}: Done."
