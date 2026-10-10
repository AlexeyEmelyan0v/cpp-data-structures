#pragma once

#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <utility>

#include "vector.hpp"

namespace exception_test_detail {

struct ThrowingValue {
  static inline int live = 0;
  static inline int operations = 0;
  static inline int throw_on = -1;

  int value;

  static void maybe_throw() {
    if (throw_on > 0 && ++operations == throw_on) {
      throw std::runtime_error("test exception");
    }
  }

  static void arm(int operation) {
    operations = 0;
    throw_on = operation;
  }

  static void disarm() noexcept {
    throw_on = -1;
    operations = 0;
  }

  explicit ThrowingValue(int x = 0): value(x) {
    maybe_throw();
    ++live;
  }

  ThrowingValue(const ThrowingValue& other): value(other.value) {
    maybe_throw();
    ++live;
  }

  ThrowingValue(ThrowingValue&& other): value(other.value) {
    maybe_throw();
    ++live;
    other.value = -1;
  }

  ThrowingValue& operator=(const ThrowingValue& other) {
    maybe_throw();
    value = other.value;
    return *this;
  }

  ThrowingValue& operator=(ThrowingValue&& other) {
    maybe_throw();
    value = other.value;
    other.value = -1;
    return *this;
  }

  ~ThrowingValue() {
    --live;
  }
};

template <typename F>
void expect_exception(int operation, F&& action) {
  ThrowingValue::arm(operation);
  bool thrown = false;

  try {
    std::forward<F>(action)();
  } catch (const std::runtime_error&) {
    thrown = true;
  }

  ThrowingValue::disarm();
  assert(thrown);
}

inline void assert_values(const Vector<ThrowingValue>& v, std::initializer_list<int> expected) {
  assert(v.size() == expected.size());
  size_t index = 0;

  for (int value : expected) {
    assert(v[index].value == value);
    ++index;
  }
}

}  // namespace exception_test_detail

inline void test_exception_copy_constructor() {
  using namespace exception_test_detail;
  assert(ThrowingValue::live == 0);

  {
    Vector<ThrowingValue> source;
    source.reserve(3);
    source.emplace_back(1);
    source.emplace_back(2);
    source.emplace_back(3);

    expect_exception(2, [&] { Vector<ThrowingValue> copy(source); });

    assert_values(source, {1, 2, 3});
    assert(ThrowingValue::live == 3);
  }

  assert(ThrowingValue::live == 0);
}

inline void test_exception_reallocation() {
  using namespace exception_test_detail;
  assert(ThrowingValue::live == 0);

  {
    Vector<ThrowingValue> v;
    v.reserve(3);
    v.emplace_back(1);
    v.emplace_back(2);
    v.emplace_back(3);
    ThrowingValue value(9);
    auto* old_data = v.data();
    const size_t old_capacity = v.capacity();

    expect_exception(2, [&] { v.reserve(10); });
    assert(v.data() == old_data && v.capacity() == old_capacity);
    assert_values(v, {1, 2, 3});
    assert(ThrowingValue::live == 4);

    expect_exception(3, [&] { v.push_back(value); });
    assert(v.data() == old_data && v.capacity() == old_capacity);
    assert_values(v, {1, 2, 3});
    assert(ThrowingValue::live == 4);

    expect_exception(2, [&] { v.emplace_back(10); });
    assert(v.data() == old_data && v.capacity() == old_capacity);
    assert_values(v, {1, 2, 3});
    assert(ThrowingValue::live == 4);

    expect_exception(2, [&] { v.resize(5); });
    assert(v.data() == old_data && v.capacity() == old_capacity);
    assert_values(v, {1, 2, 3});
    assert(ThrowingValue::live == 4);

    expect_exception(3, [&] { v.insert(v.begin() + 1, 2, value); });
    assert(v.data() == old_data && v.capacity() == old_capacity);
    assert_values(v, {1, 2, 3});
    assert(ThrowingValue::live == 4);

    expect_exception(3, [&] { v.insert(v.begin() + 1, std::move(value)); });
    assert(v.data() == old_data && v.capacity() == old_capacity);
    assert_values(v, {1, 2, 3});
    assert(ThrowingValue::live == 4);
  }

  assert(ThrowingValue::live == 0);
}

inline void test_exception_shrink_to_fit() {
  using namespace exception_test_detail;
  assert(ThrowingValue::live == 0);

  {
    Vector<ThrowingValue> v;
    v.reserve(8);
    v.emplace_back(1);
    v.emplace_back(2);
    v.emplace_back(3);
    auto* old_data = v.data();
    const size_t old_capacity = v.capacity();

    expect_exception(2, [&] { v.shrink_to_fit(); });

    assert(v.data() == old_data && v.capacity() == old_capacity);
    assert_values(v, {1, 2, 3});
    assert(ThrowingValue::live == 3);
  }

  assert(ThrowingValue::live == 0);
}

inline void test_exception_growth_without_reallocation() {
  using namespace exception_test_detail;
  assert(ThrowingValue::live == 0);

  {
    Vector<ThrowingValue> v;
    v.reserve(8);
    v.emplace_back(1);
    ThrowingValue value(9);
    auto* old_data = v.data();

    expect_exception(2, [&] { v.resize(4); });
    assert(v.data() == old_data);
    assert_values(v, {1});
    assert(ThrowingValue::live == 2);

    expect_exception(2, [&] { v.insert(v.end(), 3, value); });
    assert(v.data() == old_data);
    assert_values(v, {1});
    assert(ThrowingValue::live == 2);
  }

  assert(ThrowingValue::live == 0);
}

inline void test_exception_insert_without_reallocation() {
  using namespace exception_test_detail;
  assert(ThrowingValue::live == 0);

  {
    Vector<ThrowingValue> v;
    v.reserve(8);
    v.emplace_back(1);
    v.emplace_back(2);
    v.emplace_back(3);
    ThrowingValue value(9);
    auto* old_data = v.data();

    expect_exception(3, [&] { v.insert(v.begin(), 1, value); });
    assert(v.data() == old_data && v.size() == 3);
    assert(ThrowingValue::live == 4);

    expect_exception(4, [&] { v.insert(v.begin() + 2, 3, value); });
    assert(v.data() == old_data && v.size() == 3);
    assert(ThrowingValue::live == 4);

    expect_exception(3, [&] { v.insert(v.begin(), std::move(value)); });
    assert(v.data() == old_data && v.size() == 3);
    assert(ThrowingValue::live == 4);

    v.emplace_back(10);
    assert(v.size() == 4 && v.back().value == 10);
  }

  assert(ThrowingValue::live == 0);
}

inline void test_exception_copy_assignment() {
  using namespace exception_test_detail;
  assert(ThrowingValue::live == 0);

  {
    Vector<ThrowingValue> source;
    source.reserve(3);
    source.emplace_back(1);
    source.emplace_back(2);
    source.emplace_back(3);

    Vector<ThrowingValue> target;
    target.reserve(1);
    target.emplace_back(9);
    auto* old_data = target.data();
    const size_t old_capacity = target.capacity();

    expect_exception(2, [&] { target = source; });

    assert(target.data() == old_data && target.capacity() == old_capacity);
    assert_values(target, {9});
    assert_values(source, {1, 2, 3});
    assert(ThrowingValue::live == 4);
  }

  assert(ThrowingValue::live == 0);
}

inline void test_exception() {
  test_exception_copy_constructor();
  test_exception_reallocation();
  test_exception_shrink_to_fit();
  test_exception_growth_without_reallocation();
  test_exception_insert_without_reallocation();
  test_exception_copy_assignment();
  std::cout << "tests_exception done\n";
}
