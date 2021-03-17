#include <queue>
#include <cmath>
#include <iostream>
#include <random>
#include <stdexcept>

#include <eCP/utilities/utilities.hpp>

namespace utilities {

// Uses a set when checking for seen samples but returns a vector for convenience.
// Time complexity is O(amount), space complexity is 2*amount due to the convenience
// vector as return type.
std::vector<unsigned> get_random_unique_indexes(int amount, int container_size)
{
  if (amount < 0) { throw std::invalid_argument("Amount must be a positive number."); }
  if (amount > container_size) { throw std::invalid_argument("Amount must be less than container_size"); }

  std::vector<unsigned> collected_samples;
  collected_samples.reserve(amount);

  std::unordered_set<int> visited_samples;
  std::random_device random_seed;   // Will be used to obtain a seed for the random number engine.
  std::mt19937 generator(random_seed());  // Standard mersenne_twister_engine seeded with rd().
  int start = container_size - amount;

  for (int j = start; j < container_size; ++j) {
    std::uniform_int_distribution<> distribution(0, j);  // To-from inclusive.
    unsigned t = distribution(generator);

    std::unordered_set<int>::const_iterator iter = visited_samples.find(t);
    if (iter == visited_samples.end()) {  // Not found.
      visited_samples.insert(t);
      collected_samples.emplace_back(t);
    }
    else {
      visited_samples.insert(j);    // Found.
      collected_samples.emplace_back(j);
    }
  }
  return collected_samples;
}

std::vector<std::vector<float>> generate_descriptors(const unsigned int count, const unsigned int dimension, const unsigned int upper_bound)
{
	std::vector<std::vector<float>> vector_list;
	for (unsigned int i = 0; i < count; i++) {
		std::vector<float> point_vector;

		for (unsigned int j = 0; j < dimension; j++) {
			point_vector.push_back(static_cast<float>(rand() % upper_bound));
		}
		vector_list.push_back(point_vector);
	}

	vector_list.shrink_to_fit();
	return vector_list;
}

}
