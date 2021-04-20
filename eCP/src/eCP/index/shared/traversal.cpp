#include <eCP/index/shared/traversal.hpp>

namespace traversal {

Node* get_closest_node(const float* query, std::vector<Node>& nodes)
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

Node* find_nearest_leaf(const float* query, std::vector<Node>& nodes)
{
  Node* closest_cluster = get_closest_node(query, nodes);

  if (!closest_cluster->children.empty()) {
    return find_nearest_leaf(query, closest_cluster->children);
  }

  return closest_cluster;
}

}  // namespace traversal
