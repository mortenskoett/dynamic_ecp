#include <gtest/gtest.h>

#include <eCP/index/pre-processing.hpp>
#include <eCP/index/shared/distance.hpp>
#include <eCP/index/shared/globals.hpp>
#include <helpers/testhelpers.hpp>

// Testing compilation unit
#include <eCP/index/maintenance.cpp>

/*
 * Helpers -------------------------------------
 */

// WIP

// Index get_empty_default_index() {}

// Not used
//  std::vector<std::vector<float>> dataset{
//      {1, 1, 1}, {2, 2, 2}, {3, 3, 3}, {4, 4, 4},    {5, 5, 5},    {6, 6, 6},
//      {7, 7, 7}, {8, 8, 8}, {9, 9, 9}, {10, 10, 10}, {11, 11, 11}, {12, 12, 12},
//  };

/*
 * Creates a L1 index with 1 cluster with 3 points.
 */
Index get_test_index_A()
{
  // Arrange
  distance::set_distance_function(distance::Metric::EUCLIDEAN_OPT_UNROLL);
  globals::g_vector_dimensions = 3;
  auto policy = ReclusteringPolicy::AVERAGE;
  auto const sc = 2;

  // L1 index w. 1 cluster w. 3 points
  auto level_1 = Node{Point{{2, 2, 2}, 2}};
  level_1.points.emplace_back(Point{{3, 3, 3}, 3});
  level_1.points.emplace_back(Point{{0, 0, 0}, 0});

  auto root = Node{Point{{0, 0, 0}, 0}};
  root.children.emplace_back(level_1);

  // Create scheme
  auto scheme = ReclusteringScheme{sc, sc, policy, policy};

  // Create index.
  auto index = Index{};
  index.L = 1;
  index.size = 3;
  index.root = root;
  index.scheme = scheme;

  return index;
}

/*
 * maintenance_tests below --------------------------------------
 */

TEST(
    maintenance_tests,
    insert_given_descriptor_and_premade_12descriptor_L2_index_with_Sc2_AverageStrategy_inserts_descriptor_and_reclusters_no_index_growth)
{
  // Arrange
  distance::set_distance_function(distance::Metric::EUCLIDEAN_OPT_UNROLL);
  globals::g_vector_dimensions = 3;
  auto descriptor = Node{Point{new float[3]{42, 42, 42}, 1}};

  std::vector<std::vector<float>> dataset{
      {1, 1, 1}, {2, 2, 2}, {3, 3, 3}, {4, 4, 4},    {5, 5, 5},    {6, 6, 6},
      {7, 7, 7}, {8, 8, 8}, {9, 9, 9}, {10, 10, 10}, {11, 11, 11}, {12, 12, 12},
  };

  //  auto index = pre_processing::create_index(dataset, sc, policy, policy);
  auto index = get_test_index_A();
  auto result = testhelpers::measure_depth_from(index.root);
  ASSERT_EQ(index.size, 3);
  ASSERT_EQ(result, 1);

  // Act
  maintenance::insert(descriptor.get_leader()->descriptor, &index);

  // Assert
  EXPECT_EQ(index.size, 4);
  EXPECT_EQ(index.L, 1);
}

TEST(
    maintenance_tests,
    insert_given_descriptor_and_valid_minimal_index_Sc1_AbsoluteStrategy_inserts_descriptor_reclusters_without_growing_index)
{
  // Arrange
  distance::set_distance_function(distance::Metric::EUCLIDEAN_OPT_UNROLL);
  globals::g_vector_dimensions = 3;
  auto policy = ReclusteringPolicy::ABSOLUTE;
  auto sc = 1;
  auto descriptor = Node{Point(new float[3]{4, 4, 4}, 1)};
  std::vector<std::vector<float>> dataset = {{5, 5, 5}};

  Index* index = pre_processing::create_index(dataset, sc, 0.3, 0.3, policy, policy);
  ASSERT_EQ(index->size, 1);

  // Act
  maintenance::insert(descriptor.get_leader()->descriptor, index);

  // Assert
  EXPECT_EQ(index->size, 2);
  EXPECT_EQ(index->L, 1);
}

TEST(
    maintenance_tests,
    insert_given_descriptor_and_valid_minimal_index_Sc2_AbsoluteStrategy_inserts_descriptor_no_reclustering_no_index_growth)
{
  // Arrange
  distance::set_distance_function(distance::Metric::EUCLIDEAN_OPT_UNROLL);
  globals::g_vector_dimensions = 3;
  auto policy = ReclusteringPolicy::ABSOLUTE;
  auto sc = 2;
  auto descriptor = Node{Point(new float[3]{4, 4, 4}, 1)};
  std::vector<std::vector<float>> dataset = {{5, 5, 5}};

  Index* index = pre_processing::create_index(dataset, sc, 0.0, 0.0, policy, policy);
  ASSERT_EQ(index->size, 1);

  // Act
  maintenance::insert(descriptor.get_leader()->descriptor, index);

  // Assert
  EXPECT_EQ(index->size, 2);
}

/*
 * maintenance_helpers_tests below -------------------------------------
 */

TEST(maintenance_helpers_tests,
     collect_path_to_nearest_clusters_given_3L_index_root_returns_stack_with_4_node_ptrs)
{
  // arrange
  distance::set_distance_function(distance::Metric::EUCLIDEAN_OPT_UNROLL);
  globals::g_vector_dimensions = 3;
  unsigned sc = 2;
  auto query = Node{Point(new float[3]{4, 4, 4}, 1)};

  std::vector<std::vector<float>> dataset{
      {1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}, {10, 11, 12}, {10, 11, 12},
      {2, 2, 3}, {2, 5, 6}, {2, 8, 9}, {2, 11, 12},  {2, 11, 12},  {2, 11, 12},
  };

  // act

  // First built and validate index to validate test
  auto index = pre_processing::create_index(dataset, sc);
  auto assert_result = testhelpers::measure_depth_from(index->root);
  ASSERT_EQ(assert_result, 3);  // This must hold for this test to be valid.

  // assert
  auto root = &index->root;
  auto result = maintenance_helpers::collect_path_to_nearest_cluster(query.get_leader()->descriptor, root);
  EXPECT_EQ(result.size(), 4);
}

TEST(maintenance_helpers_tests,
     collect_path_to_nearest_clusters_given_single_root_node_returns_1_node_ptr_in_stack)
{
  // arrange
  distance::set_distance_function(distance::Metric::EUCLIDEAN_OPT_UNROLL);
  globals::g_vector_dimensions = 3;
  auto root = Node{Point(new float[3]{4, 4, 4}, 1)};

  // act
  auto result = maintenance_helpers::collect_path_to_nearest_cluster(root.get_leader()->descriptor, &root);

  // assert
  EXPECT_EQ(result.size(), 1);
}

TEST(maintenance_helpers_tests, count_descriptors_given_parent_with_2_cluster_with_2_points_each_returns_4)
{
  // Arrange
  distance::set_distance_function(distance::Metric::EUCLIDEAN_OPT_UNROLL);
  globals::g_vector_dimensions = 3;

  auto p1 = Point{new float[3]{1, 1, 1}, 1};
  auto p2 = Point{new float[3]{2, 2, 2}, 2};

  Node root{p1};
  root.children.emplace_back(Node{p2});
  root.children.emplace_back(Node{p2});

  // Add no 2 descriptor to each cluster
  root.children.front().points.emplace_back(p2);
  root.children.back().points.emplace_back(p2);

  // Act
  auto result = maintenance_helpers::count_descriptors_for_children_of(&root);
  EXPECT_EQ(result, 4);
}

TEST(maintenance_helpers_tests,
     is_cluster_reclustering_necessary_given_L2_stack_and_ABS_policy_returns_true_correct_number_of_children)
{
  // Arrange
  distance::set_distance_function(distance::Metric::EUCLIDEAN_OPT_UNROLL);
  globals::g_vector_dimensions = 3;
  unsigned sc = 2;
  auto policy = ReclusteringPolicy::ABSOLUTE;
  auto query = Node{Point(new float[3]{4, 4, 4}, 1)};

  auto p1 = Point{new float[3]{1, 1, 1}, 1};
  auto p2 = Point{new float[3]{2, 2, 2}, 2};

  Node root{p1};
  root.children.emplace_back(Node{p2});
  root.children.front().points.emplace_back(p2);
  root.children.front().points.emplace_back(p2);

  // Act
  auto stack = maintenance_helpers::collect_path_to_nearest_cluster(query.get_leader()->descriptor, &root);
  ASSERT_EQ(stack.size(), 2);

  auto cluster = stack.top();
  stack.pop();
  auto parent = stack.top();
  stack.pop();

  // Assert
  auto result = maintenance_helpers::is_cluster_reclustering_required(cluster, parent, policy, sc);

  // Total number of descriptors collected as descendants of parent.
  EXPECT_EQ(result, true);
}

TEST(
    maintenance_helpers_tests,
    is_cluster_reclustering_necessary_given_L2_Sc3_AVG_policy_with_some_cluster_larger_than_Sc_but_average_less_than_Sc_returns_false_0)
{
  // Arrange
  distance::set_distance_function(distance::Metric::EUCLIDEAN_OPT_UNROLL);
  globals::g_vector_dimensions = 3;
  unsigned sc = 2;
  auto policy = ReclusteringPolicy::AVERAGE;
  auto query = Node{Point(new float[3]{4, 4, 4}, 1)};

  auto p1 = Point{new float[3]{1, 1, 1}, 1};
  auto p2 = Point{new float[3]{2, 2, 2}, 2};

  // Average will be 5/3 = average less than sc.
  Node root{p1};
  root.children.emplace_back(Node{p2});
  root.children.emplace_back(Node{p2});
  root.children.emplace_back(Node{p2});
  root.children.front().points.emplace_back(p2);  // Has 3 points.
  root.children.front().points.emplace_back(p2);
  // Act

  // Create stack
  auto stack = maintenance_helpers::collect_path_to_nearest_cluster(query.get_leader()->descriptor, &root);
  ASSERT_EQ(stack.size(), 2);

  auto cluster = stack.top();
  stack.pop();
  auto parent = stack.top();
  stack.pop();

  // Assert
  auto result = maintenance_helpers::is_cluster_reclustering_required(cluster, parent, policy, sc);

  // Total number of descriptors collected as descendants of parent.
  EXPECT_EQ(result, false);
}

TEST(
    maintenance_helpers_tests,
    is_cluster_reclustering_necessary_given_L2_Sc3_AVG_policy_with_some_cluster_larger_than_Sc_total_larger_than_Sc_returns_true_amounof_children)
{
  // Arrange
  distance::set_distance_function(distance::Metric::EUCLIDEAN_OPT_UNROLL);
  globals::g_vector_dimensions = 3;
  unsigned sc = 2;
  auto policy = ReclusteringPolicy::AVERAGE;
  auto query = Node{Point(new float[3]{4, 4, 4}, 1)};

  auto p1 = Point{new float[3]{1, 1, 1}, 1};
  auto p2 = Point{new float[3]{2, 2, 2}, 2};

  // Average will be 5/3 = average less than sc.
  Node root{p1};
  root.children.emplace_back(Node{p2});
  root.children.emplace_back(Node{p2});
  root.children.front().points.emplace_back(p2);
  root.children.front().points.emplace_back(p2);
  root.children.front().points.emplace_back(p2);

  // Act

  // Create stack
  auto stack = maintenance_helpers::collect_path_to_nearest_cluster(query.get_leader()->descriptor, &root);
  ASSERT_EQ(stack.size(), 2);

  auto cluster = stack.top();
  stack.pop();
  auto parent = stack.top();
  stack.pop();

  // Assert
  auto result = maintenance_helpers::is_cluster_reclustering_required(cluster, parent, policy, sc);

  // Total number of descriptors collected as descendants of parent.
  EXPECT_EQ(result, true);
}

TEST(
    maintenance_helpers_tests,
    is_cluster_reclustering_necessary_given_L2_stack_and_ABS_policy_with_threshold_smaller_than_size_returns_false_0)
{
  // Arrange
  distance::set_distance_function(distance::Metric::EUCLIDEAN_OPT_UNROLL);
  globals::g_vector_dimensions = 3;
  unsigned sc = 2;
  auto policy = ReclusteringPolicy::ABSOLUTE;
  auto query = Node{Point(new float[3]{4, 4, 4}, 1)};

  auto p1 = Point{new float[3]{1, 1, 1}, 1};
  auto p2 = Point{new float[3]{2, 2, 2}, 2};

  Node root{p1};
  root.children.emplace_back(Node{p2});

  // Act

  // Create stack
  auto stack = maintenance_helpers::collect_path_to_nearest_cluster(query.get_leader()->descriptor, &root);
  ASSERT_EQ(stack.size(), 2);

  auto cluster = stack.top();
  stack.pop();
  auto parent = stack.top();
  stack.pop();

  // Assert
  auto result = maintenance_helpers::is_cluster_reclustering_required(cluster, parent, policy, sc);

  EXPECT_EQ(result, false);
}
