#include <eCP/index/maintenance.hpp>
#include <eCP/index/shared/traversal.hpp>
#include <stack>

// FIXME: THIS FUNCTION IS NOT COMPLETED -- START HERE WITH INSERTION LOGIC
std::stack<Node*> collect_parents(const float* query, std::vector<Node>& nodes)
{
  std::stack<Node*> parents;

  Node* node = traversal::get_closest_node(nodes, query);
  parents.emplace(node);

  if (!node->children.empty()) {
    return collect_parents(query, node->children);
  }

  return parents;
}

namespace maintenance {

void insert(const float* descriptor, Index& index)
{
  if (index.root.points.empty()) {
    index.root = Node{Point{descriptor, index.size}};
    index.size++;
  }
}

}  // namespace maintenance
