#include <algorithm>
#include <cmath>
#include <eCP/index/pre-processing.hpp>
#include <eCP/index/shared/distance.hpp>
#include <eCP/index/shared/globals.hpp>
#include <eCP/index/shared/traversal.hpp>
#include <eCP/utilities/utilities.hpp>

/*
 * Namespace containing testable helpers used to build the index. Compilation unit only.
 */
namespace pre_processing_helpers {

/**
 * @brief generate_leaders_indexes
 * @param dataset_size is the number of feature vectors in the the input dataset.
 * @param level_sizes is the pre-calcualted sizes of each level of the index. [0] == level L (bottom).
 * @param L is the total number of levels in the index.
 * @return a vector of L vectors each with indexes matcthing the size of the level below.
 * [0] == Level L (bottom).
 */
std::vector<std::vector<unsigned>> generate_leaders_indexes(std::size_t dataset_size,
                                                            std::vector<unsigned> level_sizes, unsigned L)
{
  // Allocate L default unsigned ints to be able to access specific indexes.
  std::vector<std::vector<unsigned>> random_leader_indexes;
  random_leader_indexes.reserve(L);

  // Pick random leaders for current level based on size of level below
  unsigned previous_level = dataset_size;

  for (unsigned i = 0; i < level_sizes.size(); ++i) {
    auto current_level = level_sizes[i];

    // Adding backwards will make index 0 == level 1.
    random_leader_indexes.emplace_back(utilities::get_random_unique_indexes(current_level, previous_level));
    previous_level = current_level;
  }

  return random_leader_indexes;
}

/**
 * @brief The IndexInitParams struct is used to pass around the initial calculated index parameters.
 */
struct IndexInitParams {
  std::vector<unsigned> level_sizes;  // Size of levels with level_sizes[0] == cluster level.
  unsigned L;                         // Depth of index.
  const unsigned lo_bound;            // Low bound used as initial size for clusters and nodes.
  const unsigned hi_bound;  // High bound used as allocation size and later as max size before reluster.

  IndexInitParams(std::vector<unsigned> level_sizes_, unsigned L_, unsigned lo_bound_, unsigned hi_bound_)
      : level_sizes(level_sizes_)
      , L(L_)
      , lo_bound(lo_bound_)
      , hi_bound(hi_bound_)
  {
  }
};

/**
 * @brief calculate_initial_index_params calculates 1) number of clusters, 2) the size of each internal node
 * level, 3) the hi/lo size span bounds used to initiate reclusterings. Might throw exception on invalid
 * input.
 * @param dataset_size is the size of the input dataset.
 * @param sc is the desired cluster and internal node size.
 * @param span is a decimal percentage 0.0 < p < 1.0 that is used to calculate the lower and the higher
 * bounded sizes of cluster and internal nodes.
 * @returns the IndexInitParam struct which contains the needed values.
 */
IndexInitParams calculate_initial_index_params(unsigned dataset_size, unsigned sc, float span)
{
  if (dataset_size < 1) {
    throw std::invalid_argument("pre_processing: Size of input dataset (n) must be larger than 0.");
  }

  if (span >= 1) {
    throw std::invalid_argument(
        "pre_processing: Range of input lo parameter must be greater than 0.0 and less than 1.0");
  }

  unsigned lo_bound =
      std::ceil(sc * (1 - span));  // Lo bound depicting the initial use of node/cluster capacity.

  if (lo_bound < 1) {
    throw std::logic_error(
        "pre_processing: Calculated lo bound must be greater than 1 after applying lo to sc.");
  }

  unsigned hi_bound = std::ceil(sc * (1 + span));  // Hi bound depicting the max size of a node/cluster.
  unsigned l = std::ceil(dataset_size / static_cast<float>(lo_bound));  // Total amount clusters.

  // Calculate the size of each level above L.
  unsigned L{1};
  unsigned current_size{l};
  std::vector<unsigned> level_sizes;
  level_sizes.emplace_back(l);

  while (current_size > lo_bound) {
    current_size = std::ceil(current_size / static_cast<float>(lo_bound));
    level_sizes.emplace_back(current_size);
    L++;
  }

  return IndexInitParams(level_sizes, L, lo_bound, hi_bound);
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
 * Finally the nested levels are added as children of a single Node which acts root.
 */
Index* create_index(const std::vector<std::vector<float>>& dataset, unsigned sc, float span,
                    ReclusteringPolicy cluster_policy, ReclusteringPolicy n_policy)

{
  // ** 1)
  const auto index_params = pre_processing_helpers::calculate_initial_index_params(dataset.size(), sc, span);

  const auto random_leader_indexes = pre_processing_helpers::generate_leaders_indexes(
      dataset.size(), index_params.level_sizes, index_params.L);

  // ** 2)

  // Used to maintain the level below when building current level
  std::vector<Node> previous_level;

  // Build levels bottom up starting with Level L which is [0].
  for (auto it = random_leader_indexes.begin(); it != random_leader_indexes.end(); ++it) {
    std::vector<Node> current_level;
    current_level.reserve(it->size());  // Allocate for already known number of leaders

    // Only for bottom level L
    if (previous_level.size() == 0) {
      for (auto index : *it) {
        // Pick from input dataset using index as Id of Point
        auto cluster = Node{Point{dataset[index].data(), index}};
        cluster.points.reserve(index_params.hi_bound);
        current_level.emplace_back(std::move(cluster));
      }
    }

    // For all levels above L (i.e. L-1...1).
    // Pick previously randomly found nodes from level below to construct current level.
    // Reconstruct Node to not copy children/points into current level.
    else {
      for (auto index : *it) {
        auto* node = &previous_level[index];
        current_level.emplace_back(Node{Point{*node->get_leader()}});
      }

      // Add all nodes from below level as children of current level
      for (auto node : previous_level) {
        traversal::get_closest_node(node.get_leader()->descriptor, current_level)
            ->children.emplace_back(std::move(node));
      }
    }
    previous_level.swap(current_level);
  }

  // ** 3)

  // Add all points from input dataset to the index.
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

  // Pick random node from top_level children to be used as root of index.
  const auto root_node_index = utilities::get_random_unique_indexes(1, previous_level.size()).front();
  auto root_point = previous_level[root_node_index].get_leader();
  auto root_node = Node{*root_point};
  root_node.children.swap(previous_level);  // Insert index levels as children of new root.

  // Create reclustering scheme based on input.
  auto scheme = ReclusteringScheme{index_params.lo_bound, index_params.hi_bound, cluster_policy, n_policy};

  return new Index{index_params.L, dataset.size(), root_node, scheme};
}

}  // namespace pre_processing
