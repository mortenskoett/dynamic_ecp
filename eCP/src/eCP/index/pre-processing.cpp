#include <algorithm>
#include <cmath>
#include <eCP/index/pre-processing.hpp>
#include <eCP/index/query-processing.hpp>
#include <eCP/index/shared/distance.hpp>
#include <eCP/index/shared/globals.hpp>
#include <eCP/utilities/utilities.hpp>
#include <iostream>

// FIXME: WIP WORK NVM THIS -- TEMP
// Code relating to hi/lo
//    const auto lo = 0.3;    // make size of sc smaller to get more clusters
//    i.e. have more and less filled clusters initially const auto hi = 0.3; //
//    enlarge size of sn with percentage to circumvent potentially over-filled
//    nodes from triggering a reclustering immediately auto sn_hi =
//    std::ceil(sn_optimal * (1 + hi)); auto sc_lo = std::ceil(sc_optimal * (1 -
//    lo)); auto sn_recalc = std::ceil(std::pow(l, 1.0/L) * (1 + hi));

// Code relating to reclustering policy
//  enum ReclusteringPolicy {AVERAGE = 1, ABSOLUTE };
//  const auto cluster_policy = ReclusteringPolicy::AVERAGE;
//  const auto leader_policy = ReclusteringPolicy::ABSOLUTE;

/*
 * Namespace containing testable helpers used to build the index. Compilation unit only.
 */
namespace pre_processing_helpers {

/**
 * @brief generate_leaders_indexes generates a set of random indexes used to
 * pick leaders from each level. Since each level will use Nodes from the level
 * below it, the return vector is generated bottom-up. E.g. the indexes for level
 * L-1 is based on the size of level L.
 * @param dataset_size is the number of feature vectors in the the input
 * dataset.
 * @param l is the pre-calculated number of clusters in the index at level L.
 * @param L is the total number of levels in the index.
 * @return a vector of L vectors containing randomly picked unsigned ints from
 * index level 1 to level L.
 */
std::vector<std::vector<unsigned>> generate_leaders_indexes(std::size_t dataset_size, unsigned l, unsigned L)
{
  std::vector<std::vector<unsigned>> random_leader_indexes(L);

  // Generate level L using pre-calculated l (number of clusters on level L)
  random_leader_indexes.back() = utilities::get_random_unique_indexes(l, dataset_size);

  // Generate levels above L, starting with L-1, ending with leader_indexes[0] is level 1.
  unsigned previous_container_size = l;
  for (unsigned i = L - 1; i > 0; --i) {
    // Calculate current level size
    unsigned level_size = ceil(pow(dataset_size, (i / (L + 1.00))));

    // Pick random leaders for current level based on size of level below
    // FIXME: Time this to see if RVO or move/copy ctors are being utilized
    random_leader_indexes[i - 1] = utilities::get_random_unique_indexes(level_size, previous_container_size);
    previous_container_size = level_size;
  }

  return random_leader_indexes;
}

/**
 * @brief get_closest_node compares each node in nodes to query and returns a
 * pointer to the closest one. It is assumed that the vector of nodes is not
 * empty.
 * @param nodes is a vector nodes.
 * @param query is the query feacture vector.
 * @return a pointer to the closest node.
 */
Node* get_closest_node(std::vector<Node>& nodes, const float* query)
{
  float max = globals::FLOAT_MAX;
  Node* closest = nullptr;

  for (Node& node : nodes) {
    const float distance = distance::g_distance_function(query, node.get_leader()->descriptor, max);

    if (distance < max) {
      max = distance;
      closest = &node;
    }
  }
  return closest;
}

/**
 * @brief find_nearest_leaf traverses the index recursively to find the leaf
 * closest to the given query vector.
 * @param query is the query vector looking for a closest cluster.
 * @param nodes is the children vector of any internal node in the index.
 * @return the nearest leaf (Node) to the given query point.
 */
Node* find_nearest_leaf(const float* query, std::vector<Node>& nodes)
{
  Node* closest_cluster = get_closest_node(nodes, query);

  if (!closest_cluster->children.empty()) {
    return find_nearest_leaf(query, closest_cluster->children);
  }

  return closest_cluster;
}

}  // namespace pre_processing_helpers

namespace pre_processing {

/*
 * This function will create an index by a 3-step process:
 * 0) Calculate l and L from input.
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
Index* create_index(const std::vector<std::vector<float>>& dataset, unsigned sc_optimal, unsigned sn_optimal)
{
  // ** 0)
  if (dataset.size() <= sc_optimal) {
    throw std::invalid_argument(
        "pre_processing: Size of input dataset (n) must be larger than input cluster size (Sc).");
  }

  if (sn_optimal <= 1) {
    throw std::invalid_argument(
        "pre_processing: Size of input internal node size (Sn) must be larger than 1.");
  }

  unsigned l = std::ceil(dataset.size() / static_cast<float>(sc_optimal));  // Total amount clusters
  unsigned L = std::ceil(std::log(l) / std::log(sn_optimal));               // Initial index depth

  if (l < 1) {
    throw std::domain_error("pre_processing: Error: Calculated value of l (number of clusters) is below 1.");
  }

  if (L < 1) {
    throw std::domain_error("pre_processing: Error: Calculated value of L (number of levels) is below 1.");
  }

  // ** 1)

  // Recalculate internal node size based on l and L
  unsigned average_internal_node_size = std::ceil(std::pow(l, 1.0 / L));

  // FIXME: Assert that RVO is being used
  const auto random_leader_indexes = pre_processing_helpers::generate_leaders_indexes(dataset.size(), l, L);

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
        cluster.points.reserve(sc_optimal);
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
        pre_processing_helpers::get_closest_node(current_level, node.get_leader()->descriptor)
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
    auto* leaf = pre_processing_helpers::find_nearest_leaf(descriptor.data(), previous_level);
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

  return new Index(L, sc_optimal, average_internal_node_size, root_node);
}

}  // namespace pre_processing
