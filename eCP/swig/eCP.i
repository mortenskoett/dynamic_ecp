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

//%include "../include/eCP/index/eCP.hpp"
//%include "../src/eCP/index/shared/data_structure.hpp"

// This can line can be tried next if there still seems to be a leak
//%typemap(newfree) Index * "free($1);";

%newobject eCP::eCP_Index;

namespace eCP {
//  Index* eCP_Index(const std::vector<std::vector<float>>& descriptors, unsigned sc, unsigned int metric);
  Index* eCP_Index(const std::vector<std::vector<float>>& descriptors, unsigned metric, unsigned sc, float span, unsigned c_policy, unsigned n_policy, bool batch_build);
  std::pair<std::vector<unsigned int>, std::vector<float>> query(Index* index, std::vector<float> query, unsigned int k, unsigned int b);
}

// clang-format on
