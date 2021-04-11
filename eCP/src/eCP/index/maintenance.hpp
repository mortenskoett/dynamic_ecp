#ifndef MAINTENANCE_HPP
#define MAINTENANCE_HPP

#include <deque>
#include <eCP/index/shared/data_structure.hpp>

namespace maintenance {

void insert(const float* descriptor, Index* const index);

Index* build_minimal_index(const float* root_descriptor, unsigned cluster_size_sc,
                           ReclusteringPolicy cluster_policy, ReclusteringPolicy node_policy);

}  // namespace maintenance

#endif  // MAINTENANCE_HPP
