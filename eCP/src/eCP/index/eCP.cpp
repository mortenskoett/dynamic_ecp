#include <cmath>
#include <eCP/index/eCP.hpp>
#include <eCP/index/maintenance.hpp>
#include <eCP/index/pre-processing.hpp>
#include <eCP/index/query-processing.hpp>
#include <eCP/index/shared/data_structure.hpp>
#include <eCP/index/shared/distance.hpp>
#include <eCP/index/shared/globals.hpp>
#include <stdexcept>

namespace eCP {
Index* eCP_Index(const std::vector<std::vector<float>>& descriptors, unsigned cluster_size, unsigned metric,
                 bool batch_build)
{
  // Set descriptor dimension globally.
  globals::g_vector_dimensions = descriptors[0].size();

  // Set distance function globally
  auto metric_type = static_cast<distance::Metric>(metric);
  distance::set_distance_function(metric_type);

  // Build index
  if (batch_build) {
    Index* index = pre_processing::create_index(descriptors, cluster_size);
    return index;
  }
  else {
    // Construct minimal index.
    std::vector<std::vector<float>> initial_node{descriptors[0]};
    Index* index = pre_processing::create_index(initial_node, cluster_size);

    // Insert the rest of the dataset.
    for (unsigned i = 1; i < descriptors.size(); ++i) {
      maintenance::insert(descriptors[i].data(), index);
    }

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
