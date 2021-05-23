#include <cassert>
#include <eCP/index/maintenance.hpp>
#include <eCP/index/shared/traversal.hpp>
#include <eCP/utilities/utilities.hpp>
#include <fstream>
#include <iostream>
#include <stack>
#include <stdexcept>

namespace maintenance_helpers {

// Store the number of reclusterings that has happened
unsigned node_reclusterings{0};
unsigned cluster_reclusterings{0};
unsigned insertions_total{0};
unsigned index_grown{0};

/**
 * @brief grow_index creates a new root node and replaces the current root in the given Index type.
 * The old root is added as child of the new.
 * @param current_root is the old root that will be substituted.
 * @param index is the index worked on and modified.
 */
void grow_index(Node* current_root, Index* const index)
{
  // Pick single random leader Node from children of current root to be the new root.
  const auto random_index = utilities::get_random_unique_indexes(1, current_root->children.size()).front();
  auto new_root_point = *current_root->children.at(random_index).get_leader();
  auto new_root = Node{new_root_point};

  // Insert new root into index.
  new_root.children.emplace_back(*current_root);  // Add old root to the children list.
  index->root = new_root;
  index->L++;
  index_grown++;
}

/**
 * @brief must_index_grow determines whether the index must grow at this point.
 * @param root is the current root node.
 * @param max_size is the maximum number of children the root can have.
 * @return
 */
bool must_index_grow(Node* root, unsigned max_size)
{
  // It only makes sense to check children of root using Absolute strategy.
  return (root->children.size() > max_size);
}

const std::vector<unsigned> generate_indexes_for_optimal_level_size(unsigned children_amount,
                                                                    unsigned optimal_size)
{
  // New level size is according to dynamic ecp SISAP paper: l = n / sn.
  const unsigned n = children_amount;
  const unsigned l = std::ceil(n / static_cast<float>(optimal_size));  // Optimal level size.
  const auto indexes = utilities::get_random_unique_indexes(l, n);
  return indexes;
}

void recluster_internal_node(Node* const node_parent, unsigned node_lo_size, unsigned node_hi_size)
{
  std::vector<Node*> children;  // Total number of children of all children under parent.
  children.reserve((node_hi_size + 1) * node_parent->children.size());  // Reserve max + 1 due to grown nodes.

  for (auto& parent_child : node_parent->children) {  // Collect all parent->children->children
    for (auto& child : parent_child.children) {
      children.emplace_back(&child);
    }
  }

  const auto indexes = generate_indexes_for_optimal_level_size(children.size(), node_lo_size);
  std::vector<Node> leaders;
  leaders.reserve(indexes.size());

  for (unsigned index : indexes) {  // Pick l random new Nodes as leaders.
    auto node{Node{*children[index]->get_leader()}};
    node.children.reserve(node_hi_size);
    leaders.emplace_back(node);
  }

  // Add each node/subtree to its nearest parent
  for (Node* node : children) {
    traversal::get_closest_node(node->get_leader()->descriptor, leaders)->children.emplace_back(*node);
  }

  node_parent->children.swap(leaders);
  node_reclusterings++;
}

void recluster_cluster(Node* const cluster_parent, unsigned cluster_lo_size, unsigned cluster_hi_size)
{
  std::vector<Point*> descriptors;  // Total number of descriptors of all children under parent.
  descriptors.reserve((cluster_hi_size + 1) * cluster_parent->children.size());  // + 1 due to grown nodes.

  for (auto& cluster : cluster_parent->children) {  // Collect all descriptors as pointers.
    for (auto& point : cluster.points) {
      descriptors.emplace_back(&point);
    }
  }

  const auto indexes = generate_indexes_for_optimal_level_size(descriptors.size(), cluster_lo_size);
  std::vector<Node> leaders;  // Revised set of leaders to substitute the children of parent.
  leaders.reserve(indexes.size());

  for (unsigned index : indexes) {  // Pick l random leaders from set of descriptors.
    auto node = Node{*descriptors[index]};
    node.points.reserve(cluster_hi_size);  // Allocate max potential size.
    leaders.emplace_back(node);
    descriptors[index] = nullptr;  // Because it will make it easier to circumvent duplicates below.
  }

  for (Point* point : descriptors) {  // Redistribute all points to the new closest clusters.
    if (point) {                      // Using nullptr to not re-add points used in Nodes above.
      traversal::find_nearest_leaf(point->descriptor, leaders)->points.emplace_back(*point);
    }
  }

  cluster_parent->children.swap(leaders);
  cluster_reclusterings++;
}

/**
 * @brief count_points_of_children counts the descriptors of all nodes on a given level.
 * Used as the typdef count_descendants_function.
 * @param parent of the clusters containing the points that will be counted.
 * @returns the total number of children.
 */
unsigned count_points_of_children(const Node* parent)
{
  unsigned descriptors_amount{0};

  for (auto& cluster : parent->children) {
    descriptors_amount += cluster.points.size();
  }
  return descriptors_amount;
}

/**
 * @brief count_nodes_of_children counts the children of all nodes on the given level.
 * Used as the typdef count_descendants_function.
 * @param parent of the nodes containing the points that will be counted.
 * @returns the total number of children.
 */
unsigned count_nodes_of_children(const Node* parent)
{
  unsigned nodes_amount{0};

  for (auto& child : parent->children) {
    nodes_amount += child.children.size();
  }
  return nodes_amount;
}

/**
 * Function pointer type definition used to be able to measure subtree size of either nodes or points.
 */
typedef unsigned (*count_descendants_function)(const Node* parent);

/**
 * @brief is_reclustering_required computes whether a reclustering is necessary based on the given
 * ReclusteringPolicy.
 * @param count_descendants_func is a function used if the AVERAGE policy is used.
 * @param number_of_children is the size (number of children) of the cluster or node being checked.
 * @param parent is the parent of the initiating node used to check if the AVERAGE policy is used.
 * @param policy is the policy used.
 * @param hi_bound is the maximum size of a node/cluster used if the ABSOLUTE policy is used.
 * @return true if a reclustering is necessary otherwise false.
 */
bool is_reclustering_required(count_descendants_function count_descendants_func, unsigned number_of_children,
                              Node* const parent, ReclusteringPolicy policy, unsigned hi_bound)
{
  switch (policy) {
    case ReclusteringPolicy::ABSOLUTE: {
      if (number_of_children > hi_bound) {
        return true;
      }
      break;
    }

    case ReclusteringPolicy::AVERAGE: {
      auto subtree_max{parent->children.size() * hi_bound};  // Theoretical max based on hi bound.
      auto subtree_actual{count_descendants_func(parent)};

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

void recursively_recluster_index(Node* initiating_node, std::stack<Node*>& path, Index* index,
                                 unsigned optimal_node_size, unsigned max_node_size)
{
  // Root is reached. Check whether the index is required to grow and recluster.
  if (path.size() == 1) {
    auto current_root = path.top();

    if (must_index_grow(current_root, max_node_size)) {
      grow_index(current_root, index);
      recluster_internal_node(&index->root, optimal_node_size, max_node_size);
    }
  }

  // Check and possibly recluster current level and ascend index.
  else {
    path.pop();  // Pop the previous parent to get next.
    auto initiating_node_parent = path.top();

    if (is_reclustering_required(count_nodes_of_children, initiating_node->children.size(),
                                 initiating_node_parent, index->scheme.node_policy, max_node_size)) {
      recluster_internal_node(initiating_node_parent, optimal_node_size, max_node_size);
      recursively_recluster_index(initiating_node_parent, path, index, optimal_node_size, max_node_size);
    }
  }
}

void initiate_index_reclustering(std::stack<Node*>& path, Index* index)
{
  const unsigned max_node_size = index->scheme.hi_bound;
  const unsigned optimal_node_size = index->scheme.lo_bound;

  auto cluster = path.top();  // The cluster that just been added to.
  path.pop();                 // Pop cluster to access leader.

  auto cluster_parent = path.top();  // Valid b/c there will always be a root node initially.

  if (is_reclustering_required(count_points_of_children, cluster->points.size(), cluster_parent,
                               index->scheme.cluster_policy, max_node_size)) {
    recluster_cluster(cluster_parent, optimal_node_size, max_node_size);
    recursively_recluster_index(cluster_parent, path, index, optimal_node_size, max_node_size);
  }
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

}  // namespace maintenance_helpers

namespace maintenance {

unsigned get_total_reclusterings()
{
  return maintenance_helpers::cluster_reclusterings + maintenance_helpers::node_reclusterings;
}
unsigned get_node_reclusterings() { return maintenance_helpers::node_reclusterings; }
unsigned get_cluster_reclusterings() { return maintenance_helpers::cluster_reclusterings; }
unsigned get_insertions_total() { return maintenance_helpers::insertions_total; }
unsigned get_times_index_has_grown() { return maintenance_helpers::index_grown; }

void append_to_csv_maintenance_metrics()
{
  auto crecl = std::to_string(maintenance::get_cluster_reclusterings());
  auto nrecl = std::to_string(maintenance::get_node_reclusterings());
  auto totalrecl = std::to_string(maintenance::get_total_reclusterings());
  auto totalinser = std::to_string(maintenance::get_insertions_total());
  auto indexgr = std::to_string(maintenance::get_times_index_has_grown());

  std::ofstream file;
  file.open("ecp_maintenance.csv", std::ios_base::app);
  //  file << "cl_recl, node_recl, total_recl, total_insert, index_gr \n";
  file << crecl << ", " << nrecl << ", " << totalrecl << ", " << totalinser << ", " << indexgr << "\n";
  file.close();
}

void print_maintenance_metrics()
{
  auto crecl = maintenance::get_cluster_reclusterings();
  auto nrecl = maintenance::get_node_reclusterings();
  auto totalrecl = maintenance::get_total_reclusterings();
  auto totalinser = maintenance::get_insertions_total();
  auto indexgr = maintenance::get_times_index_has_grown();

  std::cout << "------------------------------------" << std::endl;
  std::cout << "Maintenance metrics" << std::endl;
  std::cout << "------------------------------------" << std::endl;
  std::cout << "Total insertions: " << totalinser << std::endl;
  std::cout << "Total reclusterings: " << totalrecl << std::endl;
  std::cout << "Cluster reclusterings: " << crecl << std::endl;
  std::cout << "Node reclusterings: " << nrecl << std::endl;
  std::cout << "Index growth times: " << indexgr << std::endl;
  std::cout << std::endl;
}

void insert(const float* descriptor, Index* index)
{
  if (index->size < 1)
    throw std::invalid_argument(
        "maintenance: It is required that the index contains at least a root node in order to insert.");

  auto path = maintenance_helpers::collect_path_to_nearest_cluster(descriptor, &index->root);
  path.top()->points.emplace_back(Point{descriptor, index->size++});  // Insert descriptor and incr. size.
  maintenance_helpers::initiate_index_reclustering(path, index);
  maintenance_helpers::insertions_total++;
}

}  // namespace maintenance
