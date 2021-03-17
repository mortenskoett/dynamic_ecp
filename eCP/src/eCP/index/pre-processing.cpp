#include <cmath>
#include <algorithm>
#include <iostream>

#include <eCP/index/pre-processing.hpp>
#include <eCP/index/query-processing.hpp>
#include <eCP/index/shared/distance.hpp>
#include <eCP/index/shared/globals.hpp>
#include <eCP/utilities/utilities.hpp>

namespace pre_processing 
{
std::vector<Node> create_index(const std::vector<std::vector<float>> &dataset, unsigned int L)
{
  std::vector<std::vector<unsigned>> leader_indexes(L);   // Indexes picked randomly from input dataset used as leaders for each level

  // Computing random indexes bottom-up, leader_indexes[0] is level 1.
  unsigned container_size = dataset.size();
  for (unsigned i = L; i > 0; --i) {

    // Calculate level sizes (i.e. how many clusters for level L)
    unsigned level_size = ceil(pow(dataset.size(), (i / (L + 1.00))));

    // Pick random leaders for current level
    leader_indexes[i-1].reserve(level_size);
    leader_indexes[i-1] = utilities::get_random_unique_indexes(level_size, container_size);    // TODO: Time this to see if RVO or move/copy ctors are being utilized

    // Set to the size of current level, because indexes are found from the level below
    container_size = level_size;
  }

  std::vector<Node> previous_level;   // Used to maintain the level below when building current level

  for (auto it = leader_indexes.rbegin(); it != leader_indexes.rend(); ++it) {    // Using reverse_iterator because we need to start with bottom level
    std::vector<Node> current_level;
    current_level.reserve(it->size());

    // Only for bottom level L
    if (previous_level.size() == 0) {
      for (auto index : *it) {
        current_level.emplace_back(Node{Point{dataset[index].data(), index}});   // Pick from input dataset using index as Id of Point
      }
    }

    // For all levels above L (i.e. L-1...1)
    else {
      for (auto index : *it) {
        auto* node = &previous_level[index];   // Pick previously randomly found nodes from level below to construct current level
        current_level.emplace_back(Node{Point{node->get_leader().descriptor, node->get_leader().id}});    // To not copy children into current level
      }

      // Add all nodes from below level as children of current level
      for (auto node : previous_level) {
        get_closest_node(current_level, node.get_leader().descriptor)->children.emplace_back(node);
      }
    }

    previous_level.swap(current_level);
  }

  // Add all points from input dataset to the index incl those duplicated in the index construction.
  unsigned id{0};
  for (auto &descriptor : dataset) {
    auto *leaf = find_nearest_leaf(descriptor.data(), previous_level);
    if (id == leaf->get_leader().id) {    // Because the leader was added to the cluster when the index was built
      id ++;
      continue;
    }
    leaf->points.emplace_back(Point{descriptor.data(), id++});
  }

  return previous_level;
}

Node* find_nearest_leaf(const float* query, std::vector<Node>& nodes)
{
  Node* closest_cluster = get_closest_node(nodes, query);

  if (!closest_cluster->children.empty()) {
    return find_nearest_leaf(query, closest_cluster->children);
  }

  return closest_cluster;
}

/*
 * Assumes the given vector of nodes is not empty
 */
Node* get_closest_node(std::vector<Node>& nodes, const float* query)
{
  float max = globals::FLOAT_MAX;
  Node* closest = nullptr;

  for (Node& node : nodes) {
    const float distance = distance::g_distance_function(query, node.get_leader().descriptor);

    if (distance < max) {
      max = distance;
      closest = &node;
    }
  }
  return closest;
}

}
