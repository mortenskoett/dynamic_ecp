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
	* creates the cluster index from dataset and level parameter L
  * @param dataset is the collection of all points to be placed in the created index
  * @param L the number of levels of the index
	* @returns top_level nodes of the top level of the index
	*/
  std::vector<Node> create_index(const std::vector<std::vector<float>> &descriptors, unsigned L);

	/**
  * traverses index structure recursively to find the leaf closest to the given query vector.
  * @param query is the query vector looking for a closest cluster.
  * @param nodes is the children list of any internal node in the index.
	* @return nearest leaf node to the query point
	*/
  Node* find_nearest_leaf(const float* query, std::vector<Node>& nodes);

  /**
   * @brief get_closest_node compares each node in nodes to query
   * and returns the one that is closest.
   * The function assumes that the vector of nodes is not empty.
   * @param nodes is a vector nodes.
   * @param query is the query feacture vector.
   * @return a pointer to the closest node.
   */
  Node* get_closest_node(std::vector<Node>& nodes, const float* query);

}

#endif // PRE_PROCESSING_H
