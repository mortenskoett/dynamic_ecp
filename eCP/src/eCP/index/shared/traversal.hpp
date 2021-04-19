#ifndef TRAVERSAL_HPP
#define TRAVERSAL_HPP

#include <eCP/index/shared/data_structure.hpp>
#include <eCP/index/shared/distance.hpp>

/**
 * Namespace contains shared functions used to traverse the index.
 */
namespace traversal {

/**
 * @brief get_closest_node compares each node in nodes to query and returns a pointer to the closest one. If
 * the nodes vector is empty then a null pointer is returned.
 * @param nodes is a vector nodes.
 * @param query is the query feacture vector.
 * @return a pointer to the closest node.
 */
Node* get_closest_node(std::vector<Node>& nodes, const float* query);

/**
 * @brief find_nearest_leaf traverses the index recursively to find the leaf closest to the given query
 * vector.
 * @param query is the query vector looking for a closest cluster.
 * @param nodes is the children vector of any internal node in the index.
 * @return the nearest leaf (Node) to the given query point.
 */
Node* find_nearest_leaf(const float* query, std::vector<Node>& nodes);

}  // namespace traversal

#endif  // TRAVERSAL_HPP
