#!/usr/bin/env bash

set -e
set -o pipefail

# This script will test eCP in Ann-Benchmark.

# BRANCH="1_untouched_original_code_with_cmake_swig"
REPO_NAME="dynamic_eCP"
DATASET="random-xs-20-euclidean"
OUTPUT_DIR="mskk_results/"

echo "This script will test eCP on \"${DATASET}\" using Ann-Benchmark."
read -p "Press enter to continue"

# Go to ann-benchmarks dir
cd ../../ann-benchmarks

echo "Setup python 3.6 env inside ann-benchmarks"
python3.6 -m venv env
source env/bin/activate

echo "Running dataset: ${DATASET}..."
python run.py --dataset ${DATASET} --algorithm eCP

# Super user required
echo "Plotting dataset..."
sudo python plot.py --dataset ${DATASET}

# echo "Saving results as website..."
# sudo python create_website.py --outputdir ../mskk_results/1_untouched_original_code/
