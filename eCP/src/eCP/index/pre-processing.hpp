#ifndef PRE_PROCESSING_H
#define PRE_PROCESSING_H

#include <eCP/index/shared/data_structure.hpp>
#include <vector>

/*
 * contains functions used during the preprocessing phase.
 */
namespace pre_processing {
/**
 * @brief create_index creates the index from a given dataset and a parameter L denoting number of levels.
 * @param dataset is the collection of points to be placed in the created index.
 * @param sc_optimal is the size of each cluster and from which the number of clusters l is calculated.
 * It is also used as initial size of each internal node to calculate L.
 * @returns a pointer to the Index type on which queries can be performed.
 */
Index* create_index(const std::vector<std::vector<float>>& dataset, unsigned sc_optimal);

}  // namespace pre_processing

#endif  // PRE_PROCESSING_H
