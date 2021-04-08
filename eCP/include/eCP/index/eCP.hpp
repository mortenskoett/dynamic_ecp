#ifndef ECP_H
#define ECP_H

#include <eCP/index/shared/data_structure.hpp>
#include <vector>

/**
 * @file eCP.hpp
 * Library for the extended cluster pruning algorithm.
 *
 * @author Nikolaj Mertz
 * @author Frederik Martini
 * @author Morten Skøtt
 * @date 06/04/21
 */
namespace eCP {

/**
 * @brief eCP_Index will create an index from a data set with a given internal node size Sn and cluster size
 * Sc.
 * @param descriptors is a vector of feature descriptors that the index should be built from.
 * @param cluster_size defines the size of a cluster and is used to calculate the total number of clusters. It
 * is also used as internal node size when the depth L of the index is calculated.
 * @param metric is the used distance function for the metric space. 0 = euclidean, 1 = angular.
 * @return a pointer to the created index.
 */
Index* eCP_Index(const std::vector<std::vector<float>>& descriptors, unsigned cluster_size, unsigned metric);

/**
 * @brief query queries in the index structure and returns the k nearest points.
 * @param index is the index structure used to make queries on.
 * @param query is the query point we are looking for k-nn for.
 * @param k is the number of k-nn to return.
 * @param b is the number of clusters to search.
 * @return collection of tuples containing index in data set and distance to query point
 */
std::pair<std::vector<unsigned int>, std::vector<float>> query(Index* index, std::vector<float> query,
                                                               unsigned int k, unsigned int b);

}  // namespace eCP

#endif  // ECP_H
