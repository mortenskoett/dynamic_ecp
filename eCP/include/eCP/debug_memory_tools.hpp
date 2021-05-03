#ifndef DEBUG_MEMORY_TOOLS_HPP
#define DEBUG_MEMORY_TOOLS_HPP

#include <cstddef>
#include <eCP/data_structure.hpp>
#include <iostream>
#include <vector>

template <class T>
void print(T* prev, T* curr, ptrdiff_t diff) {
  std::cout << "prev: " << prev << "\n";
  std::cout << "curr: " << curr << "\n";
  std::cout << "diff: " << diff << "\n";
  std::cout << std::endl;
}

namespace debugging {

// template <class T>
void print_memory_distance_differences(std::vector<Node*> v) {
  if (v.size() < 1) {
    return;
  }

  Node* curr_elem = nullptr;
  Node* prev_elem = v[0];

  for (std::size_t i = 1; i < v.size(); ++i) {
    curr_elem = v[i];
    ptrdiff_t diff = (curr_elem - prev_elem) * sizeof(Node);
    print(prev_elem, curr_elem, diff);
    prev_elem = curr_elem;
  }
}

}  // namespace debugging

#endif  // DEBUG_MEMORY_TOOLS_HPP
