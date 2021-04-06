// clang-format off
%module eCP_wrapper
%{
#include "../include/eCP/index/eCP.hpp"
%}

%include "std_vector.i"
%include "std_pair.i"
%include "typemaps.i"

namespace std {
  %template(UIntVector) std::vector<unsigned int>;
  %template(FloatVector) std::vector<float>;
  %template(FloatFloatVector) std::vector<std::vector<float>>;
  %template(PairVector) std::pair<std::vector<unsigned int>, std::vector<float>>;
  %template(FloatPointerVector) std::vector<float*>;
}

namespace eCP {
  Index* eCP_Index(const std::vector<std::vector<float>>& descriptors, unsigned sc_cluster_size, unsigned sn_node_size, unsigned int metric);
  std::pair<std::vector<unsigned int>, std::vector<float>> query(Index* index, std::vector<float> query, unsigned int k, unsigned int b);
}

// clang-format on
