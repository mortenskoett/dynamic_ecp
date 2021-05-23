#! /usr/bin/env bash

# Script to install and build eCP such that it can be run locally from ann-benchmarks dir. 
# It is assumed that the 'ann-benchmarks' and the 'eCP' repositories are located in the same 
# dir './' and that this script is called from './eCP/scripts.

# Attention: It is necessary to set REPO_DIR_NAME variable to the name you have
# given the eCP repository.

# Example:
# ./ 
#   ann-benchmarks/
#   eCP/
#     scripts/

REPO_DIR_NAME="eCP"
NAME="ecp_install"

echo "${NAME}: Will generate wrapper, copy relevant ann-benchmarks files + generated wrapper."
read -p "${NAME}: Press enter to continue"

echo "${NAME}: Generating wrapper"
./gen_wrapper.sh

cd ../..

[ ! -d ann-benchmarks ] \
    && echo "${NAME}: Cloning new ann-benchmarks repo" \
    && git clone https://github.com/erikbern/ann-benchmarks

echo "${NAME}: Copy eCP necessary files"
yes | cp -rv ${REPO_DIR_NAME}/benchmarks-files/eCP.py					ann-benchmarks/ann_benchmarks/algorithms/eCP.py
yes | cp -rv ${REPO_DIR_NAME}/benchmarks-files/Dockerfile.ecp ann-benchmarks/install/Dockerfile.ecp
yes | cp -rv ${REPO_DIR_NAME}/benchmarks-files/algos.yaml			ann-benchmarks/algos.yaml
yes | cp -rv ${REPO_DIR_NAME}/benchmarks-files/algos.yaml			ann-benchmarks/algos.yaml

yes | cp -rv ${REPO_DIR_NAME}/benchmarks-files/data_export.py			ann-benchmarks/data_export.py
yes | cp -rv ${REPO_DIR_NAME}/benchmarks-files/utils.py			      ann-benchmarks/ann_benchmarks/plotting/utils.py


echo "${NAME}: Copying generated files"
yes | cp -rv ${REPO_DIR_NAME}/eCP/build/swig/_eCP_wrapper.so ann-benchmarks/
yes | cp -rv ${REPO_DIR_NAME}/eCP/build/swig/eCP_wrapper.py ann-benchmarks/

echo "${NAME}: Setup python 3.6 env inside ann-benchmarks"
cd ann-benchmarks
python3.6 -m venv env
source env/bin/activate

echo "Making export dir and export file..."
mkdir exports

echo "${NAME}: Install dependencies"
pip install -r requirements.txt

#echo "Building ONLY eCP docker image"
#python install.py --algorithm ecp

echo "${NAME}: Done."
