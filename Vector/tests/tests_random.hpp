#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include "../vector.hpp"

namespace random_tests {

inline constexpr std::uint32_t seed = 0x5EED1234;

[[noreturn]] inline void fail(std::size_t step, const char* operation, const char* detail) {
  std::cerr << "random test failed: seed=" << seed << ", step=" << step << ", operation=" << operation << ", " << detail
            << '\n';
  std::abort();
}

inline void compare(const Vector<int>& actual, const std::vector<int>& expected, std::size_t step,
                    const char* operation) {
  if (actual.size() != expected.size() || actual.capacity() < actual.size()) {
    fail(step, operation, "size or capacity differs");
  }

  for (std::size_t i = 0; i < expected.size(); ++i) {
    if (actual[i] != expected[i]) {
      fail(step, operation, "element differs");
    }
  }
}

inline std::size_t position(std::mt19937& rng, std::size_t limit) {
  return static_cast<std::size_t>(rng()) % limit;
}

}  // namespace random_tests

inline void test_random() {
  constexpr std::size_t steps = 5000;
  std::mt19937 rng(random_tests::seed);
  Vector<int> actual;
  std::vector<int> expected;

  for (std::size_t step = 0; step < steps; ++step) {
    const auto operation = rng() % 12;
    const int value = static_cast<int>(rng() % 2001) - 1000;
    const char* name = "unknown";

    switch (operation) {
      case 0: {
        name = "push_back(lvalue)";
        actual.push_back(value);
        expected.push_back(value);
        break;
      }
      case 1: {
        name = "push_back(rvalue)";
        actual.push_back(int{value});
        expected.push_back(int{value});
        break;
      }
      case 2: {
        name = "insert(count)";
        const std::size_t index = random_tests::position(rng, expected.size() + 1);
        const std::size_t count = random_tests::position(rng, 4);
        auto result = actual.insert(actual.begin() + index, count, value);
        expected.insert(expected.begin() + static_cast<std::ptrdiff_t>(index), count, value);
        if (result != actual.begin() + index) {
          random_tests::fail(step, name, "wrong return iterator");
        }
        break;
      }
      case 3: {
        name = "insert(rvalue)";
        const std::size_t index = random_tests::position(rng, expected.size() + 1);
        auto result = actual.insert(actual.begin() + index, int{value});
        expected.insert(expected.begin() + static_cast<std::ptrdiff_t>(index), int{value});
        if (result != actual.begin() + index) {
          random_tests::fail(step, name, "wrong return iterator");
        }
        break;
      }
      case 4: {
        name = "erase(one)";
        if (!expected.empty()) {
          const std::size_t index = random_tests::position(rng, expected.size());
          auto result = actual.erase(actual.begin() + index);
          expected.erase(expected.begin() + static_cast<std::ptrdiff_t>(index));
          if (result != actual.begin() + index) {
            random_tests::fail(step, name, "wrong return iterator");
          }
        }
        break;
      }
      case 5: {
        name = "erase(range)";
        if (!expected.empty()) {
          const std::size_t first = random_tests::position(rng, expected.size() + 1);
          const std::size_t last = first + random_tests::position(rng, expected.size() - first + 1);
          auto result = actual.erase(actual.begin() + first, actual.begin() + last);
          expected.erase(expected.begin() + static_cast<std::ptrdiff_t>(first),
                         expected.begin() + static_cast<std::ptrdiff_t>(last));
          if (result != actual.begin() + first) {
            random_tests::fail(step, name, "wrong return iterator");
          }
        }
        break;
      }
      case 6: {
        name = "resize";
        const std::size_t new_size = random_tests::position(rng, 40);
        actual.resize(new_size);
        expected.resize(new_size);
        break;
      }
      case 7: {
        name = "reserve";
        const std::size_t new_capacity = random_tests::position(rng, 80);
        actual.reserve(new_capacity);
        expected.reserve(new_capacity);
        break;
      }
      case 8: {
        name = "shrink_to_fit";
        actual.shrink_to_fit();
        expected.shrink_to_fit();
        break;
      }
      case 9: {
        name = "clear";
        actual.clear();
        expected.clear();
        break;
      }
      case 10: {
        name = "copy assignment";
        Vector<int> actual_copy(actual);
        std::vector<int> expected_copy(expected);
        actual = actual_copy;
        expected = expected_copy;
        break;
      }
      case 11: {
        name = "emplace_back";
        actual.emplace_back(value);
        expected.emplace_back(value);
        break;
      }
    }

    random_tests::compare(actual, expected, step, name);
  }

  std::cout << "tests_random done (seed=" << random_tests::seed << ", steps=" << steps << ")\n";
}
