#ifndef PRE_PROCESSING_H
#define PRE_PROCESSING_H

#include <vector>
#include <eCP/index/shared/data_structure.hpp>

/**
  * Contains functions for the preprocessing phase of the eCP algorithm.
  */
namespace pre_processing 
{

/**
  * @brief create_index creates the index from a dataset and a parameter L denoting number of levels.
  * @param dataset is the collection of points to be placed in the created index.
  * @param L is the number of levels the index should have.
  * @returns the topmost list of Nodes in the index.
   */
//Index* create_index(const std::vector<std::vector<float>> &descriptors, unsigned L);
std::vector<Node> create_index(const std::vector<std::vector<float>> &descriptors, unsigned L);

}

#endif // PRE_PROCESSING_H
