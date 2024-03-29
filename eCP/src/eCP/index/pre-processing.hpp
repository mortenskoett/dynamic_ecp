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
 * @brief create_index creates the index from a given dataset and parameter cluster_size defining the size
 * of a cluster size.
 * @param dataset is the collection of points to be placed in the created index.
 * @param cluster_size is the size of each cluster and from which the number of clusters l is calculated.
 * It is also used as initial size of each internal node to calculate L.
 * @param lo is a decimal percentage p < 1.0 that designates how much the clusters and nodes of the
 * index initially will be under-utilized to make room for growth.
 * Default = 0.3.
 * @param hi is a decimal percentage p that designates how much large an size overhead (over initial given
 * size) clusters and nodes will accept before a reclustering is initiated.
 * Default = 0.3.
 * @param cluster_policy is the policy used to decide when to initiate a reclustering of clusters.
 * Default=AVERAGE.
 * @param node_policy is the policy used to decide when to initiate a reclustering of internal nodes.
 * Default=ABSOLUTE. Default reclustering policies recommended in Anders' thesis pp. 38.
 * @returns a pointer to the Index type on which queries can be performed.
 */
Index* create_index(const std::vector<std::vector<float>>& dataset, unsigned cluster_size, float lo = 0.0,
                    float hi = 0.0, ReclusteringPolicy cluster_policy = ReclusteringPolicy::AVERAGE,
                    ReclusteringPolicy node_policy = ReclusteringPolicy::AVERAGE);

}  // namespace pre_processing

#endif  // PRE_PROCESSING_H
