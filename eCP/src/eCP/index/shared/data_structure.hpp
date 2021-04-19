#ifndef DATA_STRUCTURE_H
#define DATA_STRUCTURE_H

#include <cstring>
#include <eCP/index/shared/globals.hpp>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>

/*
 * Internal data structures and distance functions for the eCP algorithm.
 */

/**
 * Represents a point in high-dimensional space.
 * @param descriptor pointer to first element of feature vector>
 * @param id index in data set.
 */
struct Point {
  float* descriptor;
  unsigned long id;

  /**
   * @brief Point constructor. NB: Assumes that g_vector_dimensions is set.
   * @param descriptor_ is a pointer to the descriptor the Point should contain.
   * @param id_ is the id of the Point.
   */
  explicit Point(const float* descriptor_, unsigned long id_);

  /**
   * @brief Point constructor. NB: Assumes that g_vector_dimensions is set.
   * @param descriptor_ is a reference to a vector of floats that will be
   * copied into the point.
   * @param id_ is the id of the Point.
   */
  explicit Point(const std::vector<float> descriptor_, unsigned long id_);

  ~Point();

  // Copy constructor.
  Point(const Point& other);

  // Move constructor.
  Point(Point&& other) noexcept;

  // Copy+Move assignment operator. Notice takes concrete instance.
  Point& operator=(Point other) noexcept;

  /**
   * @brief swap follows copy-and-swap idiom. Swaps pointers instead of allocating
   * and copying the entire array.
   * See https://stackoverflow.com/questions/3279543/what-is-the-copy-and-swap-idiom.
   * @param fst is Point to swap contents to.
   * @param snd is Point to swap contents from.
   */
  friend void swap(Point& fst, Point& snd);
};

/**
 * Represents nodes and clusters in index. Will not have children at bottom level.
 * First element of points is always the representative.
 * @param children nodes at next level.
 * @param points in high-dimensional space. First element is always the node representative.
 */
struct Node {
  std::vector<Node> children;
  std::vector<Point> points;
  explicit Node();
  explicit Node(Point p);

  /**
   * @brief get_leader simply returns the leader of the Node.
   * @return a Point* which is the leader of the node in which this Point resides.
   */
  Point* get_leader();
};

/**
 * @brief The ReclusteringPolicy enum defines when a reclustering should happen.
 * If AVERAGE then a reclustering is initiated when the average size of a nodes children grows above a
 * threshold.
 * If ABSOLUTE then a reclustering is initiated whenever the number of children for any child node grows
 * beyond some threshold.
 */
enum ReclusteringPolicy { AVERAGE = 1, ABSOLUTE };

/**
 * @brief The ReclusteringScheme struct is the combined amount of information needed to decide whether a
 * reclustering should happen when an insertion has happened.
 */
struct ReclusteringScheme {
  unsigned lo_mark;                   // Lower size boundary of nodes/clusters. (min)
  unsigned hi_mark;                   // Higher size boundary of nodes/clusters. (max)
  ReclusteringPolicy cluster_policy;  // Reclustering policy for clusters.
  ReclusteringPolicy node_policy;     // Reclustering policy for internal nodes.
  explicit ReclusteringScheme();
  explicit ReclusteringScheme(unsigned lo_mark, unsigned hi_mark, ReclusteringPolicy cluster_policy_,
                              ReclusteringPolicy node_policy_);
};

/**
 * @brief Index is the definition of a constructed index with the minimal index containing a root note with a
 * single level with a single Node with a single Point. The struct is used to hold the neededed data to be
 * able to operate on the Index.
 * @param L is the depth of the index.
 * @param size is the total number of descriptors contained in the index.
 * @param scheme is the ReclusteringScheme set for the current index. Used during dynamic insertion.
 * @param root_node is the root node of the index. The children of this node are considered the first level of
 * the index L=1.
 */
struct Index {
  unsigned L;                 // Current depth
  unsigned long size;         // Number of feature descriptors contained in index.
  ReclusteringScheme scheme;  // Scheme used for reclustering.
  Node root;                  // The initial top/root node of the index.

  explicit Index();  // Possibly required by SWIG.
  explicit Index(unsigned L, unsigned long index_size, Node root_node, ReclusteringScheme scheme);
};

#endif  // DATA_STRUCTURE_H
