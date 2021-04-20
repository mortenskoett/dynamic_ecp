#include <gtest/gtest.h>

#include <eCP/index/pre-processing.hpp>
#include <eCP/index/shared/distance.hpp>
#include <eCP/index/shared/globals.hpp>
#include <helpers/testhelpers.hpp>

// Because we need to test functions only part of the compilation unit
#include <eCP/index/pre-processing.cpp>

/*
 * pre_processing_tests
 */

TEST(pre_processing_tests,
     create_index_given_root_node_sc1_and_ABSOLUTE_reclustering_policy_returns_valid_minimal_index)
{
  // Arrange
  globals::g_vector_dimensions = 3;
  auto sc = 1;
  auto hilo = 0.3;
  auto policy = ReclusteringPolicy::ABSOLUTE;

  auto descriptor = Node{Point(new float[3]{4, 4, 4}, 1)};
  std::vector<std::vector<float>> dataset = {{5, 5, 5}};

  // Act
  Index index = *pre_processing::create_index(dataset, sc, policy, policy, hilo, hilo);
  ASSERT_EQ(index.size, 1);

  auto lo_bound_res = std::ceil(sc * (1 - hilo));
  auto hi_bound_res = std::ceil(sc * (1 + hilo));

  // Assert
  EXPECT_EQ(index.L, 1);
  EXPECT_EQ(index.scheme.cluster_policy, policy);
  EXPECT_EQ(index.scheme.node_policy, policy);
  EXPECT_EQ(index.scheme.lo_bound, lo_bound_res);
  EXPECT_EQ(index.scheme.hi_bound, hi_bound_res);
  EXPECT_EQ(index.size, 1);
  EXPECT_EQ(*index.root.get_leader()->descriptor, dataset.front().front());
  EXPECT_EQ(index.root.get_leader()->id, 0);
}

TEST(pre_processing_tests,
     create_index_given_dataset_and_sc2_and_12_descriptors_returns_correct_depth_of_index)
{
  // arrange
  distance::set_distance_function(distance::Metric::EUCLIDEAN_OPT_UNROLL);
  globals::g_vector_dimensions = 3;

  // act
  std::vector<std::vector<float>> dataset{
      {1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}, {10, 11, 12}, {10, 11, 12},
      {2, 2, 3}, {2, 5, 6}, {2, 8, 9}, {2, 11, 12},  {2, 11, 12},  {2, 11, 12},
  };

  unsigned sc = 2;
  auto index = pre_processing::create_index(dataset, sc);
  auto result = testhelpers::measure_depth_from(index->root);

  // assert
  EXPECT_EQ(result, 3);
}

TEST(pre_processing_tests,
     create_index_given_dataset_with_L_2_leaders_4_points_builds_index_and_fills_clusters_with_4_points)
{
  // arrange
  unsigned sc = 2;
  distance::set_distance_function(distance::Metric::EUCLIDEAN_OPT_UNROLL);
  globals::g_vector_dimensions = 3;
  float lo = 0.3;
  float hi = 0.3;

  std::vector<std::vector<float>> dataset{
      {1, 2, 3},
      {4, 5, 6},
      {7, 8, 9},
      {10, 11, 12},
  };

  auto index_params = pre_processing_helpers::calculate_initial_index_params(dataset.size(), sc, lo, hi);

  // act
  auto leader_indexes = pre_processing_helpers::generate_leaders_indexes(
      dataset.size(), index_params.level_sizes, index_params.L);

  auto first_level = pre_processing::create_index(dataset, sc);

  auto root =
      Node{Point{{3, 3, 3}, 100}};  // FIXME: Remove this when the index uses a single Node as root node
  root.children.swap(first_level->root.children);

  auto result = testhelpers::count_points_in_clusters(root);

  // assert
  EXPECT_EQ(result, 4);
}

/*
 * pre-processing_helpers_tests
 */

TEST(pre_processing_helpers_tests,
     generate_leaders_indexes_given_dataset_12_sc2_returns_equal_correct_number_indexes_for_each_level)
{
  auto dataset_size = 12;
  auto sc = 2;
  auto L = 2;
  float lo = 0.3;
  float hi = 0.3;

  //  std::vector<unsigned> level_sizes = {6, 3, 2};
  auto level_sizes =
      pre_processing_helpers::calculate_initial_index_params(dataset_size, sc, lo, hi).level_sizes;
  ASSERT_EQ(level_sizes.size(), 3);

  auto leader_indexes = pre_processing_helpers::generate_leaders_indexes(dataset_size, level_sizes, L);

  EXPECT_EQ(leader_indexes[0].size(), 6);  // 1st level
  EXPECT_EQ(leader_indexes[1].size(), 3);  // 2nd level
  EXPECT_EQ(leader_indexes[2].size(), 2);  // 2nd level
}

TEST(pre_processing_helpers_tests,
     calculate_initial_index_params_size100_sc10_hilo_03_returns_struct_containing_correct_values)
{
  auto dataset_size = 100;
  auto sc = 10;
  float lo = 0.3;
  float hi = 0.3;

  auto res = pre_processing_helpers::calculate_initial_index_params(dataset_size, sc, lo, hi);
  auto L = res.L;
  auto lo_res = res.lo;  // 0.3
  auto hi_res = res.hi;  // 0.3

  // The values were found using excel calculations based on input.
  EXPECT_EQ(res.hi_bound, sc * (1 + hi));
  EXPECT_EQ(res.lo_bound, sc * (1 - lo));
  EXPECT_EQ(L, 2);
  EXPECT_NEAR(lo_res, lo, 0.0001);
  EXPECT_NEAR(hi_res, hi, 0.0001);
}
