#ifndef PRE_PROCESSING_H
#define PRE_PROCESSING_H

#include <eCP/index/shared/data_structure.hpp>
#include <vector>

/*
 * contains functions used during the preprocessing phase.
 */
namespace pre_processing {
/**
 * @brief create_index creates the index from a given dataset and parameter sc defining the size of a cluster
 * size.
 * @param dataset is the collection of points to be placed in the created index.
 * @param sc is the size of each cluster and from which the number of clusters l is calculated.
 * It is also used as initial size of each internal node to calculate L.
 * @returns a pointer to the Index type on which queries can be performed.
 */
Index* create_index(const std::vector<std::vector<float>>& dataset, unsigned sc);

/**
 * @brief The ReclusteringPolicy enum defines when a reclustering should happen.
 * If AVERAGE then a reclustering is initiated when the average size of a nodes children grows above a
 * threshold.
 * If ABSOLUTE then a reclustering is initiated whenever the number of children grows beyond some threshold.
 */
enum ReclusteringPolicy { AVERAGE = 1, ABSOLUTE };

}  // namespace pre_processing

#endif  // PRE_PROCESSING_H
