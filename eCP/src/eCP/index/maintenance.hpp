#ifndef MAINTENANCE_HPP
#define MAINTENANCE_HPP

#include <deque>
#include <eCP/index/shared/data_structure.hpp>

namespace maintenance {

void insert(const float* descriptor, Index* const index);

// Index* build_minimal_index(const float* root_descriptor, unsigned cluster_size,
//                           ReclusteringPolicy cluster_policy = ReclusteringPolicy::AVERAGE,
//                           ReclusteringPolicy node_policy = ReclusteringPolicy::ABSOLUTE, float lo = 0.3,
//                           float hi = 0.3);

}  // namespace maintenance

#endif  // MAINTENANCE_HPP
