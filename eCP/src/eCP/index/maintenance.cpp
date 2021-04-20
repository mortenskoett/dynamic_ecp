#include <cassert>
#include <eCP/index/maintenance.hpp>
#include <eCP/index/shared/traversal.hpp>
#include <eCP/utilities/utilities.hpp>
#include <stack>
#include <stdexcept>

namespace maintenance_helpers {

/**
 * @brief grow_index
 * @brief grow_index creates a new root node and replaces the current root in the Index type.
 * The old root is added as child of the new.
 * @param current_root is the old root that will be substituted.
 * @param index is the index worked on.
 * @return a pointer to the new root Node owned and stored by the Index.
 */
Node* grow_index(Node* current_root, Index* const index)
{
  // Pick single random leader from children of current root to be the new root.
  const auto random_index = utilities::get_random_unique_indexes(1, current_root->children.size()).front();
  auto new_root = current_root->children.at(random_index);

  // Insert new root into index.
  new_root.children.emplace_back(*current_root);  // Add old root to the children list.
  index->root = new_root;
  index->L++;

  return &index->root;
}

void recluster_internal_node(Node* const parent, unsigned desired_size)
{
  std::vector<Node*> children;  // Total number of children of all children under parent.
  children.reserve(desired_size * parent->children.size());

  for (auto& parent_child : parent->children) {  // Collect all parent->children->children
    for (auto& child : parent_child.children) {
      children.emplace_back(&child);
    }
  }

  // New level size is l = n / sn.
  const unsigned n = children.size();
  const unsigned l = std::ceil(n / static_cast<float>(desired_size));  // Floating point arith.
  const auto indexes = utilities::get_random_unique_indexes(l, n);

  std::vector<Node> leaders;
  leaders.reserve(l);

  for (unsigned index : indexes) {  // Pick l random new Nodes as leaders.
    leaders.emplace_back(Node{*children[index]->get_leader()});
  }

  // Add each node to its nearest parent
  for (Node* node : children) {
    traversal::get_closest_node(node->get_leader()->descriptor, leaders)->children.emplace_back(*node);
  }

  parent->children.swap(leaders);
}

/**
 * @brief count_children counts the children of all nodes on the given level.
 * @param parent of the nodes containing the points that will be counted.
 * @return a tuple containing the total number of children and the size of the largest children list
 * encountered.
 */
unsigned count_children_for_children_of(const Node* parent)
{
  unsigned children_amount{0};

  for (auto& child : parent->children) {
    children_amount += child.children.size();
  }
  return children_amount;
}

bool is_internal_node_reclustering_required(Node* const leader, Node* const parent, ReclusteringPolicy policy,
                                            unsigned hi_bound)
{
  switch (policy) {
    case ReclusteringPolicy::ABSOLUTE: {
      if (leader->children.size() > hi_bound) {
        return true;
      }
      break;
    }

    case ReclusteringPolicy::AVERAGE: {
      auto subtree_max{parent->children.size() * hi_bound};  // Theoretical max size based on hi bound.
      auto subtree_actual{count_children_for_children_of(parent)};

      if (subtree_actual > subtree_max) {
        return true;
      }

      break;
    }

    default:
      throw std::invalid_argument("maintenance: The given ReclusteringPolicy is invalid.");
  }
  return false;
}

void recluster_cluster(Node* const cluster_parent, unsigned desired_cluster_size)
{
  std::vector<Point*> descriptors;  // Total number of descriptors of all children under parent.
  descriptors.reserve(desired_cluster_size * cluster_parent->children.size());

  for (auto& cluster : cluster_parent->children) {  // Collect all descriptors as pointers.
    for (auto& point : cluster.points) {
      descriptors.emplace_back(&point);
    }
  }

  const unsigned n = descriptors.size();
  const unsigned l = std::ceil(n / static_cast<float>(desired_cluster_size));  // New optimal level size.
  const auto indexes = utilities::get_random_unique_indexes(l, n);

  std::vector<Node> leaders;  // Revised set of leaders to substitute the children of parent.
  leaders.reserve(l);

  for (unsigned index : indexes) {  // Pick l random leaders from set of descriptors.
    auto node = Node{*descriptors[index]};
    node.points.reserve(desired_cluster_size);
    leaders.emplace_back(node);
    descriptors[index] = nullptr;  // Because it will make it easier to circumvent duplicates below.
  }

  for (Point* point : descriptors) {  // Redistribute all points to the new closest clusters.
    if (point) {                      // Using nullptr to not re-add points used in Nodes above.
      traversal::find_nearest_leaf(point->descriptor, leaders)->points.emplace_back(*point);
    }
  }

  cluster_parent->children.swap(leaders);
}

/**
 * @brief count_descriptors counts the descriptors of all nodes on a given level.
 * @param parent of the clusters containing the points that will be counted.
 */
unsigned count_descriptors_for_children_of(const Node* parent)
{
  unsigned descriptors_amount{0};

  for (auto& cluster : parent->children) {
    descriptors_amount += cluster.points.size();
  }
  return descriptors_amount;
}

/**
 * @brief is_cluster_reclustering_required computes whether a recluster of a cluster is necessary based on the
 * given ReclusteringPolicy.
 * @param path is the stack to the cluster into which a new descriptor has been inserted.
 * @param policy is the current cluster ReclusteringPolicy.
 * @param sc_threshold is the given desired cluster size Sc of the index.
 * @returns true if a recluster is required and false otherwise.
 */
bool is_cluster_reclustering_required(Node* const cluster, Node* const parent, ReclusteringPolicy policy,
                                      unsigned hi_bound)
{
  switch (policy) {
    case ReclusteringPolicy::ABSOLUTE: {
      if (cluster->points.size() > hi_bound) {
        return true;
      }
      break;
    }

    case ReclusteringPolicy::AVERAGE: {
      auto subtree_max{parent->children.size() * hi_bound};  // Theoretical max based on hi bound.
      auto subtree_actual{count_descriptors_for_children_of(parent)};

      if (subtree_actual > subtree_max) {
        return true;
      }

      break;
    }

    default:
      throw std::invalid_argument("maintenance: The given ReclusteringPolicy is invalid.");
  }

  return false;
}

std::stack<Node*> collect_path_to_nearest_cluster(const float* query, Node* const root,
                                                  std::stack<Node*> parents = std::stack<Node*>{})
{
  parents.emplace(root);

  if (!root->children.empty()) {
    Node* closest_child = traversal::get_closest_node(query, root->children);
    return collect_path_to_nearest_cluster(query, closest_child, parents);
  }

  return parents;
}

/**
 * @brief must_index_grow determines whether the index must grow at this point.
 * @param path is the stack of pointers to each level in the index.
 * @param threshold_sn is a threshold determining a maximum internal node size.
 * @return true if the index must grow and false otherwise.
 */
bool must_index_grow(Node* root, unsigned sn_threshold)
{
  // It only makes sense to check children of root using Absolute strategy.
  return (root->children.size() > sn_threshold);
}

void orchestrate_index_reclustering(std::stack<Node*>& path, Index* index)
{
  const unsigned max_size = index->scheme.hi_bound;
  const unsigned desired_size = index->scheme.lo_bound;

  // 1 -- Handle cluster
  auto cluster = path.top();  // The cluster that just been added to.
  path.pop();                 // Pop cluster to access leader.
  auto leader = path.top();   // Valid b/c there will always be a root node initially.

  if (!is_cluster_reclustering_required(cluster, leader, index->scheme.cluster_policy, max_size)) {
    return;
  }

  recluster_cluster(leader, desired_size);

  // 2 -- Handle nodes in index
  // Recluster internal nodes above the cluster and below the root node.
  while (path.size() > 1) {
    path.pop();  // Pop the previous parent to get next.
    auto leader_parent = path.top();

    if (!is_internal_node_reclustering_required(leader, leader_parent, index->scheme.node_policy, max_size)) {
      return;
    }

    recluster_internal_node(leader_parent, desired_size);
    leader = leader_parent;  // Leader will point to the leader below on next iteration.
  }

  // 3 -- Handle root node
  // This code will only be called if the root is reached i.e. when path.size() == 1.
  assert(path.size() == 1 && "maintenance: Path stack should only contain a single root Node* here.");

  auto current_root = path.top();

  if (must_index_grow(current_root, desired_size)) {
    auto* new_root = grow_index(current_root, index);
    recluster_internal_node(new_root, desired_size);
  }
}

}  // namespace maintenance_helpers

namespace maintenance {

void insert(const float* descriptor, Index* index)
{
  if (index->size < 1)
    throw std::invalid_argument(
        "maintenance: It is required that the index contains at least a root node in order to insert.");

  auto path = maintenance_helpers::collect_path_to_nearest_cluster(descriptor, &index->root);
  path.top()->points.emplace_back(Point{descriptor, index->size++});  // Insert descriptor and incr. size.

  maintenance_helpers::orchestrate_index_reclustering(path, index);
}

}  // namespace maintenance
