#include <algorithm>
#include <cmath>
#include <eCP/index/pre-processing.hpp>
#include <eCP/index/query-processing.hpp>
#include <eCP/index/shared/distance.hpp>
#include <eCP/index/shared/globals.hpp>
#include <eCP/index/shared/traversal.hpp>
#include <eCP/utilities/utilities.hpp>
#include <iostream>

/*
 * Namespace containing testable helpers used to build the index. Compilation unit only.
 */
namespace pre_processing_helpers {

/**
 * @brief generate_leaders_indexes
 * @param dataset_size is the number of feature vectors in the the input
 * dataset.
 * @param l is the pre-calculated number of clusters in the index at level L.
 * @param sn is the internal node size.
 * @param L is the total number of levels in the index.
 * @return a vector of L vectors containing randomly picked unsigned ints from
 * index level 1 to level L.
 * @return
 */
std::vector<std::vector<unsigned>> generate_leaders_indexes(std::size_t dataset_size, unsigned l, unsigned sn,
                                                            unsigned L)
{
  std::vector<std::vector<unsigned>> random_leader_indexes(L);

  // Generate level L using pre-calculated l (number of clusters on level L)
  random_leader_indexes.back() = utilities::get_random_unique_indexes(l, dataset_size);

  // Generate levels above L, starting with L-1, ending with leader_indexes[0] is level 1.
  unsigned previous_container_size = l;
  for (unsigned i = L - 1; i > 0; --i) {
    unsigned level_size = ceil(pow(sn, i));  // Levels are based on input argument sn

    // Pick random leaders for current level based on size of level below
    random_leader_indexes[i - 1] = utilities::get_random_unique_indexes(level_size, previous_container_size);
    previous_container_size = level_size;
  }

  return random_leader_indexes;
}

/**
 * @brief The IndexInitParams struct is used to pass around the initial calculated index parameters.
 */
struct IndexInitParams {
  unsigned l;         // Amount of clusters.
  unsigned L;         // Depth of index.
  const unsigned sc;  // Size of a cluster.
  const unsigned sn;  // Size of an internal node.
  const float lo;     // Percentage increasing amount of clusters by decreasing size of a cluster.
  const float hi;     // Percentage decreasing the likelihood of a recluster.
};

/**
 * @brief calculate_initial_index_params calculates the needed values when the index is initialized.
 * These values are calculated according to the eCP paper as well as the SISAP dynamic eCP paper.
 * Might throw exception on invalid input.
 * @param dataset_size is the size of the input dataset.
 * @param sc is the desired cluster size.
 * @returns the IndexInitParam struct which contains the needed values.
 */
IndexInitParams calculate_initial_index_params(unsigned dataset_size, unsigned sc)
{
  if (dataset_size <= sc) {
    throw std::invalid_argument(
        "pre_processing: Size of input dataset (n) must be larger than input cluster size (Sc).");
  }

  const float lo = 0.3;  // Make sc smaller to get more clusters i.e. have more and less filled clusters.
  const float hi = 0.3;  // Enlarge sn to circumvent nodes from triggering a reclustering immediately.

  // Using sc for both initial cluster and internal node size.
  unsigned sc_lo = std::ceil(sc * (1 - lo));
  unsigned sn_hi = std::ceil(sc * (1 + hi));

  unsigned l = std::ceil(dataset_size / static_cast<float>(sc_lo));  // Total amount clusters.
  unsigned L = std::ceil(std::log(l) / std::log(sn_hi));             // Initial index depth.

  if (l < 1) {
    throw std::domain_error("pre_processing: Error: Calculated value of l (number of clusters) is below 1.");
  }

  if (L < 1) {
    throw std::domain_error("pre_processing: Error: Calculated value of L (number of levels) is below 1.");
  }

  // Recalculate internal node size based on l and L.
  unsigned sn_recalc = std::ceil(std::pow(l, 1.0 / L) * (1 + hi));

  return IndexInitParams{l, L, sc_lo, sn_recalc, lo, hi};
}

}  // namespace pre_processing_helpers

namespace pre_processing {

/*
 * This function will create an index by a 3-step process:
 * 1) Generate random indexes used to pick leaders for each level.
 * 2) Built index bottom-up. Each level is constructed from the level
 * below. Initially the bottom level L is constructed from the input dataset.
 * Then level L-1 is constructed similarly, but from nodes from level L. Now all
 * nodes from level L are added to level L-1 using the distance function to
 * place them correctly. This process repeats for all levels up to and inclusive
 * level 1.
 * 3) All input vectors are added to the index except those that are
 * already there due to the Node constructor adding the leader to the Points vector.
 */
Index* create_index(const std::vector<std::vector<float>>& dataset, unsigned cluster_size_sc,
                    ReclusteringPolicy cluster_policy, ReclusteringPolicy node_policy)
{
  // ** 1)
  const auto index_params =
      pre_processing_helpers::calculate_initial_index_params(dataset.size(), cluster_size_sc);
  const auto random_leader_indexes = pre_processing_helpers::generate_leaders_indexes(
      dataset.size(), index_params.l, index_params.sn, index_params.L);

  // ** 2)

  // Used to maintain the level below when building current level
  std::vector<Node> previous_level;

  // Using reverse_iterator because we need to start with bottom level
  for (auto it = random_leader_indexes.rbegin(); it != random_leader_indexes.rend(); ++it) {
    std::vector<Node> current_level;
    current_level.reserve(it->size());  // Allocate for already known number of leaders

    // Only for bottom level L
    if (previous_level.size() == 0) {
      for (auto index : *it) {
        // Pick from input dataset using index as Id of Point
        auto cluster = Node{Point{dataset[index].data(), index}};
        cluster.points.reserve(index_params.sc);
        current_level.emplace_back(std::move(cluster));
      }
    }

    // For all levels above L (i.e. L-1...1)
    // Pick previously randomly found nodes from level below to construct current level
    // Reconstruct Node to not copy children/points into current level
    else {
      for (auto index : *it) {
        auto* node = &previous_level[index];
        current_level.emplace_back(Node{Point{*node->get_leader()}});
      }

      // Add all nodes from below level as children of current level
      for (auto node : previous_level) {
        traversal::get_closest_node(current_level, node.get_leader()->descriptor)
            ->children.emplace_back(std::move(node));
      }
    }
    previous_level.swap(current_level);
  }

  // ** 3)

  // Add all points from input dataset to the index
  // FIXME: Optional optimization: Use a set to contain id's.
  unsigned id{0};
  for (auto& descriptor : dataset) {
    auto* leaf = traversal::find_nearest_leaf(descriptor.data(), previous_level);
    // Only add if id was not added to as leader of the cluster when the index was built
    if (id != leaf->get_leader()->id) {
      leaf->points.emplace_back(Point{descriptor.data(), id});
    }
    id++;
  }

  // Pick random node from top_level children to be used as root of index
  const auto root_node_index = utilities::get_random_unique_indexes(1, previous_level.size()).front();
  auto root_point = previous_level[root_node_index].get_leader();
  auto root_node = Node{*root_point};
  root_node.children.swap(previous_level);  // Insert index levels as children of root

  return new Index(index_params.L, dataset.size(), index_params.sc, index_params.sn, cluster_policy,
                   node_policy, root_node);
}

}  // namespace pre_processing
