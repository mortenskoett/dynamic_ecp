#include <ittnotify.h>

#include <eCP/debugging/debug_tools.hpp>
#include <eCP/index/eCP.hpp>
#include <eCP/utilities/utilities.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
  // clang-format off

  /* For vtune params */
  const int metric = 0;  // Distance metric - 0 = euclidean - 1 = angular - 2 = euclidean with early halting
  const int k = 100;     // number points to return
  const int b = 40;      // number clusters to search
  int p = 500'000;       // number of vectors
  const int d = 128;     // dimensions of vector
  const int r = 1000;    // upper bound of generated vectors
  const int qs = 10000;  // queries to make on created index
  const unsigned sc = 70;  // optimal cluster size
  const unsigned sn = 70;   // optimal node/leader size

  /* For debugging params */
//    const int metric = 0;   // Distance metric - 0 = euclidean - 1 = angular - 2 = euclidean with early halting
//    const int k = 2;        // number points to return
//    const int b = 2;        // number clusters to search
//    unsigned p = 12;        // number of vectors
//    const int d = 128;      // dimensions of vector
//    const int r = 1000;     // upper bound of generated vectors
//    const int qs = 15;      // queries to make on created index
//    const unsigned sc = 2;  // optimal cluster size
//    const unsigned sn = 2;  // optimal node/leader size

  // clang-format on

  /* Setup ITTAPI instrumentation domain */
  __itt_domain* domain_build = __itt_domain_create("ECP.BENCHMARKING.BUILD");
  __itt_domain* domain_query = __itt_domain_create("ECP.BENCHMARKING.QUERY");
  __itt_string_handle* handle_build = __itt_string_handle_create("ecp_build");
  __itt_string_handle* handle_query = __itt_string_handle_create("ecp_query");

  /* Generate dummy data */
  std::vector<std::vector<float>> S;
  std::vector<std::vector<float>> queries;

  /* If hdf5 file path is given as program argument */
  if (argc == 2) {
    std::cout << "Running with hdf5 file: " << argv[1] << std::endl;
    std::string file = std::string(argv[1]);
    std::string dataset = "train";
    S = utilities::load_hdf5_file(file, dataset);
    dataset = "test";
    queries = utilities::load_hdf5_file(file, dataset);
    p = S.size();
  }
  else {
    std::cout << "Generating " << p << " descriptors with " << d << " dimensions and values up to " << r
              << "\n";
    S = utilities::generate_descriptors(p, d, r);
    queries = utilities::generate_descriptors(qs, d, r);
    std::cout << "Done generating dummy data." << std::endl;
  }

  /* Index build instrumentation */
  __itt_task_begin(domain_build, __itt_null, __itt_null, handle_build);
  Index* index = eCP::eCP_Index(S, sc, sn, metric);
  __itt_task_end(domain_build);

  /* Query instrumentation */
  __itt_task_begin(domain_query, __itt_null, __itt_null, handle_query);
  for (auto& q : queries) {
    auto result = eCP::query(index, q, k, b);
    //        debugging::print_query_results(result, q, k, S);   // debugging
  }
  __itt_task_end(domain_query);

  /* Debugging */
  // debugging::print_clusters(index->top_level);      // debugging
  // debugging::print_index_levels(index->top_level);  // debugging

  /* Clean up */
  delete index;
  std::cout << "eCP run OK.\n";
  std::cout << "dataset size: " << p << "\n";
  return 0;
}
