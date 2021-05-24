#! /usr/bin/env bash

# Script to be copied into ann-benchmarks dir to automatically run and move around benchmarks
# for absolute convenience. Uncomment dataset to be run.

NAME="ecp_run_local"  # Script name.

# ALGO_NAME="eCP" # WARNING: Must match in algos.yaml + eCP.py.
ALGOS=(eCP eCP-2 eCP-3)

# DATASET="random-xs-20-euclidean"
DATASET="glove-25-angular"
# DATASET="sift-128-euclidean"

echo "${NAME}: Will run benchmarks on ${ALGOS[*]}. First run 'ecp_install.sh'. Dataset: $DATASET."
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
# python run.py --local --algorithm ${ALGO_NAME} --dataset ${DATASET}

for algo in "${ALGOS[@]}";do
  python run.py --local --algorithm "$algo" --dataset ${DATASET}
done

echo "${NAME}: Creating plots and website"
python plot.py --dataset ${DATASET}

echo "${NAME}: Exporting data to ./exports"
python data_export.py --output exports/exports.csv

echo "${NAME}: Averaging AB data samples"
python cleanup.py

echo "${NAME}: Moving samples into separate directory"
mkdir mskk_benchmark
mv -v ecp_clusters_bulk.csv   mskk_benchmark/ecp_clusters_bulk.csv
mv -v ecp_clusters_incr.csv   mskk_benchmark/ecp_clusters_incr.csv 
mv -v ecp_coordinates.csv     mskk_benchmark/ecp_coordinates.csv 
mv -v ecp_general_stats.csv   mskk_benchmark/ecp_general_stats.csv 
mv -v ecp_maintenance.csv     mskk_benchmark/ecp_maintenance.csv 
mv -v ecp_nodes_bulk.csv      mskk_benchmark/ecp_nodes_bulk.csv 
mv -v ecp_nodes_incr.csv      mskk_benchmark/ecp_nodes_incr.csv
mv -v exports/exports.csv     mskk_benchmark/exports.csv

# mkdir -p website
# python create_website.py --outputdir website --scatter --latex

echo "${NAME}: Done."
