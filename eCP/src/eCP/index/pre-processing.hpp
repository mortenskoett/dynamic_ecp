#ifndef PRE_PROCESSING_H
#define PRE_PROCESSING_H

#include <eCP/index/shared/data_structure.hpp>
#include <vector>

/*
 * contains functions used during the preprocessing phase.
 */
namespace pre_processing {

/**
 */

/**
 * @brief create_index creates the index from a given dataset and parameter sc defining the size
 * of a cluster size.
 * @param dataset is the collection of points to be placed in the created index.
 * @param sc is the size of each cluster and from which the number of clusters l is calculated.
 * It is also used as initial size of each internal node to calculate L.
 * @param span is a decimal percentage 0.0 < p < 1.0 that is used to calculate the lower and the higher
 * bounded sizes of cluster and internal nodes.
 * @param cluster_policy is the policy used to decide when to initiate a reclustering of clusters.
 * Default=ABSOLUTE.
 * @param n_policy is the policy used to decide when to initiate a reclustering of internal nodes.
 * Default=ABSOLUTE. Default reclustering policies recommended in Anders' thesis pp. 38.
 * @returns a pointer to the Index type on which queries can be performed.
 */
Index* create_index(const std::vector<std::vector<float>>& dataset, unsigned sc, float span = 0.0,
                    ReclusteringPolicy cluster_policy = ABSOLUTE, ReclusteringPolicy n_policy = ABSOLUTE);

}  // namespace pre_processing

#endif  // PRE_PROCESSING_H
