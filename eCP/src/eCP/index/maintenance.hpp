#ifndef MAINTENANCE_HPP
#define MAINTENANCE_HPP

#include <deque>
#include <eCP/index/shared/data_structure.hpp>

namespace maintenance {

/**
 * @brief insert inserts a given descriptor to the index and initiates a reclustering if necessary.
 * @param descriptor is the descriptor to insert. It it assumed that the descriptor has the exact same
 * dimensionality as the dataset the index was initially constructed from.
 * @param index is the index to insert into.
 */
void insert(const float* descriptor, Index* const index);

}  // namespace maintenance

#endif  // MAINTENANCE_HPP
