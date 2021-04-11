#include <gtest/gtest.h>

#include <eCP/index/shared/distance.hpp>
#include <eCP/index/shared/globals.hpp>
#include <helpers/testhelpers.hpp>

// Because we need to test functions only part of the compilation unit
#include <eCP/index/pre-processing.cpp>

/*
 * pre_processing_tests
 */

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
  EXPECT_EQ(result, 2);
}

TEST(pre_processing_tests,
     create_index_given_dataset_with_L_2_leaders_4_points_builds_index_and_fills_clusters_with_4_points)
{
  // arrange
  auto dataset_size = 4;
  unsigned L = 2;
  unsigned sc = 2;
  distance::set_distance_function(distance::Metric::EUCLIDEAN_OPT_UNROLL);
  globals::g_vector_dimensions = 3;
  unsigned l = 3;

  std::vector<std::vector<float>> dataset{
      {1, 2, 3},
      {4, 5, 6},
      {7, 8, 9},
      {10, 11, 12},
  };

  // act
  auto leader_indexes = pre_processing_helpers::generate_leaders_indexes(dataset_size, l, sc, L);

  auto first_level = pre_processing::create_index(dataset, sc);

  auto root =
      Node{Point{{3, 3, 3}, 100}};  // FIXME: Remove this when the index uses a single Node as root node
  root.children.swap(first_level->root.children);

  auto result = testhelpers::count_points_in_clusters(root);

  // assert
  EXPECT_EQ(result, 4);
}

// FIXME: All of these tests need to be re-implemented

/* Helpers */
// std::vector<Node> get_empty_index(unsigned int L = 2)
//{
//    std::vector<Point> S = {
//        Point(new float[3] {1, 1, 1}, 0),
//        Point(new float[3] {4, 4, 4}, 1),
//        Point(new float[3] {2, 2, 2}, 2),
//        Point(new float[3] {12, 12, 12}, 3),
//        Point(new float[3] {3, 3, 3}, 4),
//        Point(new float[3] {9, 9, 9}, 5),
//        Point(new float[3] {5, 5, 5}, 6),
//        Point(new float[3] {20, 20, 20}, 7),
//        Point(new float[3] {17, 17, 17}, 8),
//        Point(new float[3] {7, 7, 7}, 9),
//        Point(new float[3] {10, 10, 10}, 10),
//    };

//    return pre_processing::create_index(S, L);
//};

/* Tests */

// TEST(pre_processing_tests, create_index_bottom_up_creates_empty_datastructure) {
//    distance::set_distance_function(distance::Metrics::EUCLIDEAN_OPT_UNROLL);
//    globals::g_vector_dimensions = 3;

//    std::vector<Point> S = {
//        Point(new float[3] {1, 1, 1}, 0),
//        Point(new float[3] {4, 4, 4}, 1),
//        Point(new float[3] {2, 2, 2}, 2),
//        Point(new float[3] {3, 3, 3}, 3),
//        Point(new float[3] {9, 9, 9}, 4),
//        Point(new float[3] {6, 6, 6}, 5)
//    };
//    unsigned int L = 1;

//    auto root = pre_processing::create_index(S, L);

//    EXPECT_TRUE(root.size() != 0);

//    for (Node &cluster : root) {
//        EXPECT_TRUE(cluster.children.size() == 0);
//        EXPECT_TRUE(cluster.points.size() == 1);
//    }
//}

// FIXME: Rewrite this test
// TEST(pre_processing_tests, insert_points_given_empty_index_inserts_points) {
//    distance::set_distance_function(distance::Metrics::EUCLIDEAN_OPT_UNROLL);
//    globals::g_vector_dimensions = 3;

//    std::vector<Node> root = get_empty_index(1);

//    std::vector<Point> descriptors = {
//        Point(new float[3] {2, 2, 2}, 0),
//        Point(new float[3] {9, 9, 9}, 1),
//        Point(new float[3] {4, 4, 4}, 2),
//        Point(new float[3] {1, 1, 1}, 3),
//        Point(new float[3] {12, 12, 12}, 4),
//        Point(new float[3] {17, 17, 17}, 5),
//        Point(new float[3] {5, 5, 5}, 6),
//        Point(new float[3] {3, 3, 3}, 7),
//        Point(new float[3] {20, 20, 20}, 8),
//        Point(new float[3] {7, 7, 7}, 9),
//        Point(new float[3] {10, 10, 10}, 10)
//    };

//    std::vector<Node> actual = pre_processing::insert_points(root, descriptors);

//    for (auto cluster : actual) {
//        for (auto leaf : cluster.children) {
//            EXPECT_TRUE(leaf.points.size() != 0);
//        }
//    }
//}

/*
 * pre-processing_helpers_tests
 */

TEST(pre_processing_helpers_tests,
     generate_leaders_indexes_given_dataset_12_sc2_returns_correct_number_for_each_level)
{
  auto dataset_size = 12;
  auto l = 6;
  auto L = 2;
  auto sn = 4;

  auto leader_indexes = pre_processing_helpers::generate_leaders_indexes(dataset_size, l, sn, L);

  EXPECT_EQ(leader_indexes[0].size(), 4);  // 1st level
  EXPECT_EQ(leader_indexes[1].size(), 6);  // 2nd level
}

TEST(pre_processing_helpers_tests,
     calculate_initial_index_params_size100_sc10_hilo_03_returns_struct_containing_correct_values)
{
  auto dataset_size = 100;
  auto sc = 10;

  auto res = pre_processing_helpers::calculate_initial_index_params(dataset_size, sc);
  auto l = res.l;
  auto L = res.L;
  auto sc_res = res.sc;
  auto sn_res = res.sn;
  auto lo = res.lo;  // 0.3
  auto hi = res.hi;  // 0.3

  // The values were found using excel calculations based on input.
  EXPECT_EQ(l, 15);
  EXPECT_EQ(L, 2);
  EXPECT_EQ(sc_res, 7);
  EXPECT_EQ(sn_res, 6);  // initially sn == sc but sn is recalculated in FUT
  EXPECT_NEAR(lo, 0.3, 0.0001);
  EXPECT_NEAR(hi, 0.3, 0.0001);
}
