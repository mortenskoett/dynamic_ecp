#include <gtest/gtest.h>

#include <eCP/index/shared/distance.hpp>
#include <eCP/index/shared/globals.hpp>
#include <helpers/testhelpers.hpp>
#include <eCP/index/shared/traversal.hpp>

/*
 * traversal_tests
 */

TEST(traversal_tests, get_closest_node_returns_closest_cluster)
{
  distance::set_distance_function(distance::Metric::EUCLIDEAN_OPT_UNROLL);
  globals::g_vector_dimensions = 3;

  std::vector<Node> clusters{
      Node{Point(new float[3]{1, 1, 1}, 0)},
      Node{Point(new float[3]{4, 4, 4}, 1)},
      Node{Point(new float[3]{7, 7, 7}, 2)},
      Node{Point(new float[3]{8, 8, 8}, 3)},
  };

  float* query = new float[3]{3, 3, 3};

  float expected[3] = {4, 4, 4};
  Node* actual = traversal::get_closest_node(clusters, query);

  EXPECT_EQ(*actual->points[0].descriptor, *expected);
}

TEST(traversal_tests, get_closest_node_given_query_in_clusters_returns_same)
{
  distance::set_distance_function(distance::Metric::EUCLIDEAN_OPT_UNROLL);
  globals::g_vector_dimensions = 3;

  std::vector<Node> clusters = {
      Node{Point{new float[3]{1, 1, 1}, 0}},
      Node(Point(new float[3]{4, 4, 4}, 1)),
      Node(Point(new float[3]{7, 7, 7}, 2)),
      Node(Point(new float[3]{8, 8, 8}, 3)),
  };

  float* query = new float[3]{8, 8, 8};

  float expected[3] = {8, 8, 8};
  Node* actual = traversal::get_closest_node(clusters, query);

  EXPECT_EQ(*actual->points[0].descriptor, *expected);
}

TEST(traversal_tests, find_nearest_leaf_finds_nearest_cluster_in_2_level_index)
{
  distance::set_distance_function(distance::Metric::EUCLIDEAN_OPT_UNROLL);
  globals::g_vector_dimensions = 3;

  std::vector<Node> clusters = {
      Node{Point{new float[3]{10, 10, 10}, 10}},
      Node(Point(new float[3]{100, 100, 100}, 100)),
      Node(Point(new float[3]{1000, 1000, 1000}, 1000)),
      Node(Point(new float[3]{10'000, 10'000, 10'000}, 10'000)),
  };

  Point p1{Point{new float[3]{2, 2, 2}, 2}};
  Point p2{Point{new float[3]{900, 900, 900}, 900}};
  Node node1{p1};
  Node node2{p2};

  node2.children = clusters;
  std::vector<Node> l1 = {node1, node2};

  float* query = new float[3]{999, 999, 999};
  float* expected = clusters[2].points[0].descriptor;

  Node* actual = traversal::find_nearest_leaf(query, l1);

  EXPECT_EQ(*actual->points[0].descriptor, *expected);
}
