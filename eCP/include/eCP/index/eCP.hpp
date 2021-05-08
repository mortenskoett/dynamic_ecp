#ifndef ECP_H
#define ECP_H

#include <eCP/index/shared/data_structure.hpp>
#include <vector>

/**
 * @file eCP.hpp
 * Library API for the extended cluster pruning algorithm.
 *
 * @author Nikolaj Mertz
 * @author Frederik Martini
 * @author Morten Skøtt
 * @date 06/04/21
 */
namespace eCP {

/**
 * @brief eCP_Index will create an index from a dataset using the given metric and desired cluster size.
 * @param descriptors is a vector of feature descriptors that the index should be built from.
 * @param sc defines the desired size of clusters and nodes.
 * @param span defines the min/max span size of an internal node and cluster in the index.
 * @param c_policy is the policy used to decide when to recluster clusters. 1=Average, 2=Absolute.
 * @param n_policy is the policy used to decide when to recluster internal nodes. 1=Average, 2=Absolute.
 * @param metric is the utilized distance function of the metric space. See the @ref{Metric} type.
 * @param batch_build designates when true that the index should be bulk built from the input dataset and when
 * false that the index should be built incrementally.
 * @returns a pointer to the constructed index.
 */
Index* eCP_Index(const std::vector<std::vector<float>>& descriptors, unsigned metric, unsigned sc, float span,
                 unsigned c_policy, unsigned n_policy, bool batch_build = true);

/**
 * @brief insert will insert a descriptor into the index. It is assumed that the given descriptor is of equal
 * dimensionality to what the index already contains.
 * @param descriptor is the given feature descriptor to insert.
 * @param index is the index to insert into.
 */
void insert(const float* descriptor, Index* const index);

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
