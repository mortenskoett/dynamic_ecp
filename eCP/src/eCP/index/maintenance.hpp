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

// Get metrics on number of reclusterings.
unsigned get_insertions_total();
unsigned get_total_reclusterings();
unsigned get_node_reclusterings();
unsigned get_cluster_reclusterings();
unsigned get_times_index_has_grown();

void append_to_csv_maintenance_metrics();
void print_maintenance_metrics();

}  // namespace maintenance

#endif  // MAINTENANCE_HPP
