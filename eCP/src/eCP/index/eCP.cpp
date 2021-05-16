#include <cmath>
#include <eCP/debugging/debug_tools.hpp>
#include <eCP/index/eCP.hpp>
#include <eCP/index/maintenance.hpp>
#include <eCP/index/pre-processing.hpp>
#include <eCP/index/query-processing.hpp>
#include <eCP/index/shared/data_structure.hpp>
#include <eCP/index/shared/distance.hpp>
#include <eCP/index/shared/globals.hpp>
#include <stdexcept>

ReclusteringPolicy convert_unsigned_to_reclustering_policy(unsigned val)
{
  if (val < 1 || val > 2) {
    throw std::invalid_argument("Invalid unsigned given. Cannot be converted to ReclusteringStrategy.");
  }

  return static_cast<ReclusteringPolicy>(val);
}

namespace eCP {

Index* eCP_Index(const std::vector<std::vector<float>>& descriptors, unsigned metric, unsigned sc, float span,
                 unsigned c_policy, unsigned n_policy, float percentage)
{
  if (percentage < 0.0 || percentage > 100.0) {
    throw std::invalid_argument("Invalid unsigned given. Percentage must be between 0-100.");
  }

  // Set descriptor dimension globally.
  globals::g_vector_dimensions = descriptors[0].size();

  // Set distance function globally
  auto metric_type = static_cast<distance::Metric>(metric);
  distance::set_distance_function(metric_type);

  // Convert input to internally used strategies
  auto cluster_policy = convert_unsigned_to_reclustering_policy(c_policy);
  auto node_policy = convert_unsigned_to_reclustering_policy(n_policy);

  // Bulk/incrementally constructed index.
  if (percentage < 100) {
    auto amount_incr = std::floor((descriptors.size() / 100) * percentage);
    auto amount_bulk = descriptors.size() - amount_incr;

    // Create bulk loaded part of index.
    Index* index =
        pre_processing::create_index(descriptors, sc, span, cluster_policy, node_policy, amount_bulk);

    debugging::print_index_vector_info("Clusters sizes after bulk insertion: L=" + std::to_string(index->L) +
                                           "," + "Size=" + std::to_string(index->size),
                                       "", debugging::count_cluster_sizes(index));
    debugging::print_index_vector_info("Nodes sizes after bulk insertion:", "",
                                       debugging::count_node_sizes(index));

    // Insert the rest of the dataset.
    for (unsigned i = amount_bulk; i < descriptors.size(); ++i) {
      maintenance::insert(descriptors[i].data(), index);
    }

    debugging::print_index_vector_info("Cluster sizes after incremental insertion:", "",
                                       debugging::count_cluster_sizes(index));
    debugging::print_index_vector_info("Nodes sizes after incremental insertion:", "",
                                       debugging::count_node_sizes(index));

    maintenance::print_maintenance_metrics();

    return index;
  }

  // 100% incrementally constructed.
  else {
    Index* index = pre_processing::create_index(descriptors, sc, span, cluster_policy, node_policy, 1);

    // Insert the rest of the dataset.
    for (unsigned i = 1; i < descriptors.size(); ++i) {
      maintenance::insert(descriptors[i].data(), index);
    }

    debugging::print_index_vector_info("Clusters sizes after incremental insertion:", "",
                                       debugging::count_cluster_sizes(index));
    debugging::print_index_vector_info("Nodes sizes after incremental insertion:", "",
                                       debugging::count_node_sizes(index));

    maintenance::print_maintenance_metrics();

    return index;
  }
}

void insert(const float* descriptor, Index* const index)
{
  // Inserts descriptor into index.
  maintenance::insert(descriptor, index);
}

std::pair<std::vector<unsigned int>, std::vector<float>> query(Index* index, std::vector<float> query,
                                                               unsigned int k, unsigned int b)
{
  // internal data structure uses float pointer instead of vectors
  float* q = query.data();

  auto nearest_points = query_processing::k_nearest_neighbors(index->root.children, q, k, b, index->L);

  // unzip since id are only needed for ANN-Benchmarks
  std::vector<unsigned int> nearest_indexes = {};
  std::vector<float> nearest_dist = {};

  for (auto it = std::make_move_iterator(nearest_points.begin()),
            end = std::make_move_iterator(nearest_points.end());
       it != end; ++it) {
    nearest_indexes.push_back(it->first);
    nearest_dist.push_back(it->second);
  }

  return make_pair(nearest_indexes, nearest_dist);
}

}  // namespace eCP
