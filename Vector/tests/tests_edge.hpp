#pragma once

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "../vector.hpp"

inline void test_edge() {
  {
    Vector<int> empty;
    empty.reserve(8);
    assert(empty.size() == 0 && empty.capacity() >= 8);
    empty.shrink_to_fit();
    assert(empty.size() == 0 && empty.capacity() == 0);
    assert(empty.begin() == empty.end());
  }

  {
    Vector<int> values{1, 2, 3};
    values.reserve(8);
    auto* storage = values.data();
    auto result = values.erase(values.begin() + 1, values.begin() + 1);
    assert(result == values.begin() + 1);
    assert(values.data() == storage && values.size() == 3);
    assert(values[0] == 1 && values[1] == 2 && values[2] == 3);

    Vector<int>& same = values;
    values = std::move(same);
    assert(values.data() == storage && values.size() == 3);
    assert(values[0] == 1 && values[1] == 2 && values[2] == 3);

    values.erase(values.begin(), values.end());
    assert(values.empty() && values.capacity() >= 8);
    values.push_back(4);
    assert(values.size() == 1 && values.front() == 4);
  }

  {
    Vector<int> values;
    values.insert(values.begin(), 42);
    assert(values.size() == 1 && values.front() == 42);
    values.clear();
    values.insert(values.begin(), 3, 7);
    assert(values.size() == 3 && values[0] == 7 && values[1] == 7 && values[2] == 7);
  }

  {
    Vector<int> values{10};
    const Vector<int>& const_values = values;
    bool caught = false;
    try {
      (void)const_values.at(1);
    } catch (const std::out_of_range&) {
      caught = true;
    }
    assert(caught);
  }

  std::cout << "tests_edge done\n";
}
