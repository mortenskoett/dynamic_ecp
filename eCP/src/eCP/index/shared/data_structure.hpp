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
  unsigned id;

  /**
   * @brief Point constructor. NB: Assumes that g_vector_dimensions is set.
   * @param descriptor_ is a pointer to the descriptor the Point should contain.
   * @param id_ is the id of the Point.
   */
  explicit Point(const float* descriptor_, unsigned id_);

  /**
   * @brief Point constructor. NB: Assumes that g_vector_dimensions is set.
   * @param descriptor_ is a reference to a vector of floats that will be
   * copied into the point.
   * @param id_ is the id of the Point.
   */
  explicit Point(const std::vector<float> descriptor_, unsigned id_);

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

// FIXME: WIP on a better architecture for policies
// struct Policy {
//  unsigned threshold;  // Sn/Sc
//  unsigned max_size;   // Max size for any single node or cluster.
//  virtual bool is_reclustering_necessary(Node* const current) = 0;
//};

// struct AverageClusterPolicy : public Policy {
// public:
//  virtual bool is_reclustering_necessary(Node* const current) override;
//};

// struct AbsoluteClusterPolicy : public Policy {
// public:
//  virtual bool is_reclustering_necessary(Node* const current) override;
//};

//// struct InternalNodePolicy : public Policy {
////};

// AbsoluteClusterPolicy a = AbsoluteClusterPolicy{};
// Policy& b = a;

/**
 * @brief Index is the definition of a constructed or partially constructed index. The struct is used to hold
 * the neededed data to be able to operate on the Index.
 * @param L is the depth of the index.
 * @param index_size is the total number of descriptors contained in the index.
 * @param sc_ is the cluster size, often used also as internal node size sn.
 * @param cluster_policy_ is the policy used to decide when to initiate a reclustering of clusters.
 * @param node_policy_ is the policy used to decide when to initiate a reclustering of internal nodes.
 * @param root_node is the root node of the index. The children of this node are considered the first level of
 * the index L=1.
 */
struct Index {
  unsigned L;                         // Current depth
  unsigned sc;                        // Cluster size
  unsigned sn;                        // Internal node size
  unsigned size;                      // Number of feature descriptors contained in index.
  ReclusteringPolicy cluster_policy;  // Reclustering policy for clusters.
  ReclusteringPolicy node_policy;     // Reclustering policy for internal nodes.
  Node root;

  explicit Index();  // Possibly required by SWIG.
  explicit Index(unsigned sc_, ReclusteringPolicy cluster_policy_, ReclusteringPolicy node_policy_,
                 Node root_node);
  explicit Index(unsigned L, unsigned index_size, unsigned sc_, unsigned sn_,
                 ReclusteringPolicy cluster_policy_, ReclusteringPolicy node_policy_, Node root_node);
};

#endif  // DATA_STRUCTURE_H
