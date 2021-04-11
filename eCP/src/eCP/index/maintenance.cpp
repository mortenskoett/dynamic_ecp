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

void recluster_internal_node(Node* const parent, unsigned sn_threshold)
{
  std::vector<Node*> children;  // Total number of children of all children under parent.
  children.reserve(sn_threshold * parent->children.size());

  for (auto& parent_child : parent->children) {  // Collect all parent->children->children
    for (auto& child : parent_child.children) {
      children.emplace_back(&child);
    }
  }

  // New level size is l = n / sn.
  const unsigned n = children.size();
  const unsigned l = std::ceil(n / static_cast<float>(sn_threshold));  // Floating point arith.
  const auto indexes = utilities::get_random_unique_indexes(l, n);

  std::vector<Node> leaders;
  leaders.reserve(l);

  for (unsigned index : indexes) {  // Pick l random new Nodes as leaders.
    leaders.emplace_back(*children[index]);
  }

  // Add each node to its nearest parent
  for (Node* node : children) {
    // FIXME: CHECK IF THE ADDRESS IS DIFFERENT IE DATA IS GETTING ALLOCATED HERE.
    traversal::find_nearest_leaf(node->get_leader()->descriptor, leaders)->children.emplace_back(*node);
  }

  parent->children.swap(leaders);
}

/**
 * @brief The DescendantData struct is used to carry a tuple of two unsigned.
 */
struct DescendantData {
  unsigned amount;
  unsigned max_value;
};

/**
 * @brief count_children counts the children of all nodes on the given level.
 * @param parent of the nodes containing the points that will be counted.
 * @return a tuple containing the total number of children and the size of the largest children list
 * encountered.
 */
DescendantData count_children(const Node* parent)
{
  unsigned children_amount{0};
  unsigned max_size{0};

  for (auto& child : parent->children) {
    auto current = child.children.size();
    children_amount += child.children.size();

    if (current > max_size) {
      max_size = current;
    }
  }
  return DescendantData{children_amount, max_size};
}

bool is_internal_node_reclustering_required(Node* const leader, Node* const parent, ReclusteringPolicy policy,
                                            unsigned sn_threshold)
{
  switch (policy) {
    case ReclusteringPolicy::ABSOLUTE: {
      if (leader->children.size() > sn_threshold) {
        return true;
      }
      break;
    }

    case ReclusteringPolicy::AVERAGE: {
      DescendantData children_data = {count_children(parent)};
      auto all_children{children_data.amount};
      auto max_children_in_single_node{children_data.max_value};

      float average_children_per_cluster =
          all_children / static_cast<float>(parent->children.size());  // Cast for float arithm.

      if (average_children_per_cluster > sn_threshold) {
        return true;
      }

      if (max_children_in_single_node > sn_threshold * 2) {
        return true;
      }

      break;
    }

    default:
      throw std::invalid_argument("maintenance: The given ReclusteringPolicy is invalid.");
  }
  return false;
}

void recluster_cluster(Node* const cluster_parent, unsigned sc_optimal_cluster_size)
{
  std::vector<Point*> descriptors;  // Total number of descriptors of all children under parent.
  descriptors.reserve(sc_optimal_cluster_size * cluster_parent->children.size());

  for (auto& cluster : cluster_parent->children) {  // Collect all descriptors as pointers.
    for (auto& point : cluster.points) {
      descriptors.emplace_back(&point);
    }
  }

  const unsigned n = descriptors.size();
  const unsigned l = std::ceil(n / static_cast<float>(sc_optimal_cluster_size));  // Floating point arith.
  const auto indexes = utilities::get_random_unique_indexes(l, n);

  std::vector<Node> leaders;  // Revised set of leaders to substitute the children of parent.
  leaders.reserve(l);

  for (unsigned index : indexes) {  // Pick l random leaders from set of descriptors.
    auto node = Node{*descriptors[index]};
    node.points.reserve(sc_optimal_cluster_size);
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
 * @return a tuple containing the total amount of descriptors and the size of the largest cluster.
 */
DescendantData count_descriptors(const Node* parent)
{
  unsigned descriptors_amount{0};
  unsigned max_size{0};

  for (auto& cluster : parent->children) {
    auto current = cluster.points.size();
    descriptors_amount += current;

    if (current > max_size) {
      max_size = current;
    }
  }
  return DescendantData{descriptors_amount, max_size};
}

/**
 * @brief is_cluster_reclustering_required computes whether a recluster is necessary based on the given
 * ReclusteringPolicy. Will pop top element from the stack.
 * @param path is the stack to the cluster into which a new descriptor has been inserted.
 * @param policy is the current cluster ReclusteringPolicy.
 * @param sc_threshold is the given desired cluster size Sc of the index.
 * @returns true if a recluster is required and false otherwise.
 */
bool is_cluster_reclustering_required(Node* const cluster, Node* const parent, ReclusteringPolicy policy,
                                      unsigned sc_threshold)
{
  switch (policy) {
    case ReclusteringPolicy::ABSOLUTE: {
      if (cluster->points.size() > sc_threshold) {
        return true;
      }
      break;
    }

    case ReclusteringPolicy::AVERAGE: {
      DescendantData descriptor_data = {count_descriptors(parent)};
      auto all_descriptors{descriptor_data.amount};
      auto max_points_in_single_cluster{descriptor_data.max_value};

      float average_number_per_cluster =
          all_descriptors / static_cast<float>(parent->children.size());  // Cast for float arithm.

      if (average_number_per_cluster > sc_threshold) {
        return true;
      }

      if (max_points_in_single_cluster > sc_threshold * 2) {
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
    Node* closest_child = traversal::get_closest_node(root->children, query);
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
  // It only makes sense to check root children using Absolute strategy.
  return (root->children.size() > sn_threshold);
}

void orchestrate_index_reclustering(std::stack<Node*>& path, Index* index)
{
  // 1 -- Handle cluster
  auto cluster = path.top();  // Initial cluster added to.
  path.pop();                 // Pop cluster to access leader.
  auto leader = path.top();   // Valid b/c there will always be a root node initially.

  if (!is_cluster_reclustering_required(cluster, leader, index->cluster_policy, index->sc)) {
    return;
  }

  recluster_cluster(leader, index->sc);

  // 2 -- Handle nodes in index
  // Recluster internal nodes above the cluster and below the root node.
  while (path.size() > 1) {
    path.pop();  // Pop the previous parent to get next.
    auto leader_parent = path.top();

    if (!is_internal_node_reclustering_required(leader, leader_parent, index->node_policy, index->sn)) {
      return;
    }

    recluster_internal_node(leader_parent, index->sn);
    leader = leader_parent;  // Leader will point to the leader below on next iteration.
  }

  // 3 -- Handle root node
  // This code will only be called if the root is reached i.e. when path.size() == 1.
  assert(path.size() == 1 && "maintenance: Path stack should only contain a single root Node* here.");

  auto current_root = path.top();
  if (must_index_grow(current_root, index->sn)) {
    auto new_root = grow_index(current_root, index);
    recluster_internal_node(new_root, index->sn);
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

Index* build_minimal_index(const float* root_descriptor, unsigned cluster_size_sc,
                           ReclusteringPolicy cluster_policy, ReclusteringPolicy node_policy)
{
  Node initial_node = Node{Point{root_descriptor, 0}};
  auto index = new Index{cluster_size_sc, cluster_policy, node_policy, initial_node};
  index->root.children.emplace_back(initial_node);
  return index;
}

}  // namespace maintenance
