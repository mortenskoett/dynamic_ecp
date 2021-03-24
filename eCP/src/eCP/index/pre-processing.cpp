#include <cmath>
#include <algorithm>
#include <iostream>

#include <eCP/index/pre-processing.hpp>
#include <eCP/index/query-processing.hpp>
#include <eCP/index/shared/distance.hpp>
#include <eCP/index/shared/globals.hpp>
#include <eCP/utilities/utilities.hpp>

// Aliases
using VectorFloat2D = std::vector<std::vector<float>>;
using VectorUnsigned2D = std::vector<std::vector<unsigned>>;

namespace pre_processing
{

/**
 * @brief get_closest_node compares each node in nodes to query and returns a pointer to the closest one.
 * It is assumed that the vector of nodes is not empty.
 * @param nodes is a vector nodes.
 * @param query is the query feacture vector.
 * @return a pointer to the closest node.
 */
Node* get_closest_node(std::vector<Node>& nodes, const float* query)
{
  float max = globals::FLOAT_MAX;
  Node* closest = nullptr;

  for (Node& node : nodes) {
    const float distance = distance::g_distance_function(query, node.get_leader()->descriptor);

    if (distance < max) {
      max = distance;
      closest = &node;
    }
  }
  return closest;
}

/**
 * @brief find_nearest_leaf traverses the index recursively to find the leaf closest to the given query vector.
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

/**
 * @brief generate_leaders_indexes generates a set of random indexes used to pick leaders
 * from each level. Since each level will use Nodes from the level below it, the indexes
 * for level L-1 is based on the size of level L.
 * @param dataset_size is the number of feature vectors in the the input dataset.
 * @param L is the number of levels to generate leader indexes for.
 * @return a vector of vectors containing randomly picked unsigned ints in the range
 * 0..size(level below).
 */
VectorUnsigned2D generate_leaders_indexes(const std::size_t dataset_size, const unsigned L)
{
  VectorUnsigned2D random_leader_indexes(L);   // Indexes picked randomly from input dataset used as leaders for each level

  // Computing random indexes bottom-up, leader_indexes[0] is level 1.
  unsigned container_size = dataset_size;
  for (unsigned i = L; i > 0; --i) {

    // Calculate level sizes (i.e. how many clusters for level L)
    unsigned level_size = ceil(pow(dataset_size, (i / (L + 1.00))));

    // Pick random leaders for current level
    random_leader_indexes[i-1].reserve(level_size);
    random_leader_indexes[i-1] = utilities::get_random_unique_indexes(level_size, container_size);    // FIXME: Time this to see if RVO or move/copy ctors are being utilized

    // Set to the size of current level, because indexes are found from the level below
    container_size = level_size;
  }

  return random_leader_indexes;
}

/**
 * @brief build_index build the index bottom-up. Each level is constructed from the level below.
 * Initially the bottom level L is constructed from the input dataset. Then level L-1 is constructed
 * similarly, but from nodes from level L. Now all nodes from level L are added to level L-1 using
 * the distance function to place them correctly. This process repeats for all levels up to and
 * inclusive level 1.
 * @param dataset is the set of feature vectors to initiate the index with.
 * @param leader_indexes is a vector of vecors containing leaders indexes for each level.
 * @param cluster_alloc_size is the size each internal node/cluster will be allocated with.
 * @return a vector representing the topmost level in the index, L=1.
 */
std::vector<Node> build_index(const VectorFloat2D &dataset, const VectorUnsigned2D leader_indexes,
                              const unsigned cluster_alloc_size)
{
  std::vector<Node> previous_level;   // Used to maintain the level below when building current level

  for (auto it = leader_indexes.rbegin(); it != leader_indexes.rend(); ++it) {    // Using reverse_iterator because we need to start with bottom level
    std::vector<Node> current_level;
    current_level.reserve(it->size());    // Allocate for already known number of leaders

    // Only for bottom level L
    if (previous_level.size() == 0) {
      for (auto index : *it) {
        auto cluster = Node{Point{dataset[index].data(), index}};   // Pick from input dataset using index as Id of Point
        cluster.points.reserve(cluster_alloc_size);    // Allocate initial size of cluster
        current_level.emplace_back(cluster);
      }
    }

    // For all levels above L (i.e. L-1...1)
    else {
      for (auto index : *it) {
        auto* node = &previous_level[index];   // Pick previously randomly found nodes from level below to construct current level
        current_level.emplace_back(Node{node->points[0]});    // Reconstruct Point to not copy children/points into current level
      }

      // Add all nodes from below level as children of current level
      for (auto node : previous_level) {
        get_closest_node(current_level, node.get_leader()->descriptor)->children.emplace_back(node);
      }
    }
    previous_level.swap(current_level);
  }

  return previous_level;
}

/**
 * @brief fill_clusters fills the clusters of the index with points from the input
 * dataset. All input vectors are added to the index except those that are already
 * there due to how Nodes are currently instantiated.
 * It is pass-by-value to utilize RVO.
 * @param index is the index worked on.
 * @param dataset is the dataset to insert into index.
 * @returns a vector representing the top level but filled with points from dataset.
 */
std::vector<Node> fill_clusters(std::vector<Node> top_level, const VectorFloat2D &dataset)
{
  // Add only points not already in there from the index construction.
  unsigned id{0};
  for (auto &descriptor : dataset) {
    auto *leaf = find_nearest_leaf(descriptor.data(), top_level);    // FIXME: Optional optimization: Use a set to contain all leader id's. Then we don't have to call distance function for those.

    if (id != leaf->get_leader()->id) {    // Because the leader was added to the cluster when the index was built
      leaf->points.emplace_back(Point{descriptor.data(), id});
    }
    id++;
  }
  return top_level;
}

/*
 * Create index API function.
 * It is pass-by-value to utilize RVO.
 */
std::vector<Node> create_index(const VectorFloat2D &dataset, unsigned int L)
{
  const auto random_leader_indexes = generate_leaders_indexes(dataset.size(), L);
  const unsigned int average_cluster_size = ceil(pow(dataset.size(), (1.00 / (L + 1.00))));    // Each cluster will represent on average, n^( 1/(L+1) ) points

  std::vector<Node> top_level = build_index(dataset, random_leader_indexes, average_cluster_size);
  std::vector<Node> top_level_filled = fill_clusters(top_level, dataset);   // Utilizes RVO

  return top_level_filled;
}

}   // pre_processing
