#ifndef PRE_PROCESSING_H
#define PRE_PROCESSING_H

#include <vector>
#include <eCP/index/shared/data_structure.hpp>

 /*
  * Dontains functions for the preprocessing phase of the eCP algorithm.
  */
namespace pre_processing 
{
/**
  * @brief create_index creates the index from a dataset and a parameter L denoting number of levels.
  * @param dataset is the collection of points to be placed in the created index.
  * @param L is the number of levels the index should have.
  * @returns the topmost list of Nodes in the index.
   */
  std::vector<Node> create_index(const std::vector<std::vector<float>> &descriptors, unsigned L);

  /**
  * @brief find_nearest_leaf traverses the index recursively to find the leaf closest to the given query vector.
  * @param query is the query vector looking for a closest cluster.
  * @param nodes is the children vector of any internal node in the index.
  * @return the nearest leaf (Node) to the given query point.
   */
  Node* find_nearest_leaf(const float* query, std::vector<Node>& nodes);

  /**
   * @brief get_closest_node compares each node in nodes to query and returns the one that is closest.
   * The function assumes that the vector of nodes is not empty.
   * @param nodes is a vector nodes.
   * @param query is the query feacture vector.
   * @return a pointer to the closest node.
   */
  Node* get_closest_node(std::vector<Node>& nodes, const float* query);

}

#endif // PRE_PROCESSING_H
