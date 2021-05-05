// clang-format off
%module eCP_wrapper
%{
#include "../include/eCP/index/eCP.hpp"
#include "../src/eCP/index/shared/data_structure.hpp"
%}

%include std_vector.i
%include std_pair.i
%include typemaps.i

namespace std {
  %template(UIntVector) std::vector<unsigned int>;
  %template(FloatVector) std::vector<float>;
  %template(FloatFloatVector) std::vector<std::vector<float>>;
  %template(PairVector) std::pair<std::vector<unsigned int>, std::vector<float>>;
  %template(FloatPointerVector) std::vector<float*>;
}

// %include "../include/eCP/index/eCP.hpp"
// %include "../src/eCP/index/shared/data_structure.hpp"

%newobject eCP::eCP_Index;

namespace eCP {
  Index* eCP_Index(std::vector<std::vector<float>> descriptors, unsigned int L, unsigned int metric);
  std::pair<std::vector<unsigned int>, std::vector<float>> query(Index* index, std::vector<float> query, unsigned int k, unsigned int b);
}

// clang-format on
