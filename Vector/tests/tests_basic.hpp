#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

#include "vector.hpp"

inline void test_push_back() {
  Vector<int> v;

  v.push_back(10);
  v.push_back(20);

  assert(v.size() == 2);
  assert(v[0] == 10);
  assert(v[1] == 20);
}

inline void test_insert() {
  Vector<int> v;

  v.push_back(1);
  v.push_back(2);
  v.insert(v.begin(), 4);
  v.insert(v.begin() + 2, 5);
  v.insert(v.end(), 2, 6);

  assert(v.size() == 6);
  assert(v[0] == 4);
  assert(v[1] == 1);
  assert(v[2] == 5);
  assert(v[3] == 2);
  assert(v[4] == 6);
  assert(v[5] == 6);
}

inline void test_insert_without_reallocation() {
  Vector<int> v{1, 2, 3, 4, 5};
  v.reserve(12);
  int* old_data = v.data();
  const size_t old_capacity = v.capacity();
  int value = 7;

  auto it = v.insert(v.begin() + 1, 2, value);

  assert(it == v.begin() + 1);
  assert(v.data() == old_data);
  assert(v.capacity() == old_capacity);
  assert(v.size() == 7);
  assert(v[0] == 1 && v[1] == 7 && v[2] == 7);
  assert(v[3] == 2 && v[4] == 3 && v[5] == 4 && v[6] == 5);

  Vector<int> aliased{1, 2, 3};
  aliased.reserve(10);
  old_data = aliased.data();

  it = aliased.insert(aliased.begin() + 2, 3, aliased[0]);

  assert(it == aliased.begin() + 2);
  assert(aliased.data() == old_data);
  assert(aliased.size() == 6);
  assert(aliased[0] == 1 && aliased[1] == 2);
  assert(aliased[2] == 1 && aliased[3] == 1 && aliased[4] == 1 && aliased[5] == 3);
}

inline void test_insert_zero() {
  Vector<int> v{1, 2, 3};
  int* old_data = v.data();
  const size_t old_capacity = v.capacity();

  auto it = v.insert(v.begin() + 1, 0, v[0]);

  assert(it == v.begin() + 1);
  assert(v.data() == old_data);
  assert(v.capacity() == old_capacity);
  assert(v.size() == 3);
  assert(v[0] == 1 && v[1] == 2 && v[2] == 3);
}

inline void test_insert_rvalue() {
  Vector<std::unique_ptr<int>> v;
  v.reserve(4);
  v.push_back(std::make_unique<int>(1));
  v.push_back(std::make_unique<int>(3));
  auto* old_data = v.data();

  auto it = v.insert(v.begin() + 1, std::make_unique<int>(2));

  assert(v.data() == old_data);
  assert(it == v.begin() + 1);
  assert(v.size() == 3);
  assert(*v[0] == 1 && *v[1] == 2 && *v[2] == 3);

  v.shrink_to_fit();
  it = v.insert(v.end(), std::make_unique<int>(4));

  assert(it == v.begin() + 3);
  assert(v.size() == 4);
  assert(*v[0] == 1 && *v[1] == 2 && *v[2] == 3 && *v[3] == 4);
}

inline void test_constructor() {
  Vector<int> v(3, 7);

  assert(v.size() == 3);
  assert(v[0] == 7);
  assert(v[1] == 7);
  assert(v[2] == 7);
}

inline void test_initializer_list() {
  Vector<int> v{1, 2, 3, 4};

  assert(v.size() == 4);
  assert(v[0] == 1);
  assert(v[1] == 2);
  assert(v[2] == 3);
  assert(v[3] == 4);
}

inline void test_pop_back() {
  Vector<int> v{1, 2, 3};

  v.pop_back();

  assert(v.size() == 2);
  assert(v.back() == 2);
}

inline void test_resize() {
  Vector<int> v{1, 2, 3};

  v.resize(5);

  assert(v.size() == 5);
  assert(v[0] == 1);
  assert(v[1] == 2);
  assert(v[2] == 3);
  assert(v[3] == 0);
  assert(v[4] == 0);

  v.resize(2);

  assert(v.size() == 2);
  assert(v[0] == 1);
  assert(v[1] == 2);
}

inline void test_erase() {
  Vector<int> v{1, 2, 3, 4, 5};

  auto it = v.erase(v.begin() + 1, v.begin() + 3);

  assert(v.size() == 3);
  assert(v[0] == 1);
  assert(v[1] == 4);
  assert(v[2] == 5);
  assert(it == v.begin() + 1);
  assert(*it == 4);
}

inline void test_clear() {
  Vector<int> v{1, 2, 3};
  size_t old_capacity = v.capacity();

  v.clear();

  assert(v.empty());
  assert(v.size() == 0);
  assert(v.capacity() == old_capacity);
}

inline void test_copy() {
  Vector<int> a{1, 2, 3};
  Vector<int> b(a);

  assert(a == b);

  b[0] = 10;

  assert(a[0] == 1);
  assert(b[0] == 10);
}

inline void test_move() {
  Vector<int> a{1, 2, 3};
  Vector<int> b(std::move(a));

  assert(b.size() == 3);
  assert(b[0] == 1);
  assert(b[1] == 2);
  assert(b[2] == 3);

  assert(a.size() == 0);
  assert(a.capacity() == 0);
  assert(a.data() == nullptr);
}

inline void test_at() {
  Vector<int> v{1, 2, 3};

  assert(v.at(1) == 2);

  bool thrown = false;

  try {
    v.at(3);
  } catch (const std::out_of_range&) {
    thrown = true;
  }

  assert(thrown);
}

inline void test_reserve() {
  Vector<int> v{1, 2, 3};

  v.reserve(10);

  assert(v.size() == 3);
  assert(v.capacity() >= 10);
  assert(v[0] == 1);
  assert(v[1] == 2);
  assert(v[2] == 3);
}

inline void test_shrink_to_fit() {
  Vector<int> v{1, 2, 3};

  v.reserve(20);
  assert(v.capacity() >= 20);

  v.shrink_to_fit();

  assert(v.size() == 3);
  assert(v.capacity() == 3);
  assert(v[0] == 1);
  assert(v[1] == 2);
  assert(v[2] == 3);
}

inline void test_front_back() {
  Vector<int> v{10, 20, 30};

  assert(v.front() == 10);
  assert(v.back() == 30);

  v.front() = 1;
  v.back() = 3;

  assert(v[0] == 1);
  assert(v[2] == 3);
}

inline void test_iterators() {
  Vector<int> v{1, 2, 3, 4};

  int sum = 0;

  for (auto it = v.begin(); it != v.end(); ++it) {
    sum += *it;
  }

  assert(sum == 10);
}

inline void test_reverse_iterators() {
  Vector<int> v{1, 2, 3};

  auto it = v.rbegin();

  assert(*it == 3);
  ++it;
  assert(*it == 2);
  ++it;
  assert(*it == 1);
}

inline void test_copy_assignment() {
  Vector<int> a{1, 2, 3};
  Vector<int> b{10, 20};

  b = a;

  assert(b.size() == 3);
  assert(b[0] == 1);
  assert(b[1] == 2);
  assert(b[2] == 3);

  b[0] = 100;
  assert(a[0] == 1);
}

inline void test_copy_assignment_reuses_capacity() {
  Vector<int> src{1, 2, 3};
  Vector<int> dst{9};
  dst.reserve(10);
  int* old_data = dst.data();
  const size_t old_capacity = dst.capacity();

  dst = src;

  assert(dst.data() == old_data);
  assert(dst.capacity() == old_capacity);
  assert(dst.size() == 3);
  assert(dst[0] == 1 && dst[1] == 2 && dst[2] == 3);

  Vector<int> smaller{7};
  dst = smaller;

  assert(dst.data() == old_data);
  assert(dst.size() == 1);
  assert(dst[0] == 7);
}

inline void test_swap() {
  Vector<int> a{1, 2};
  Vector<int> b{3, 4, 5};
  a.reserve(8);
  b.reserve(10);
  int* a_data = a.data();
  int* b_data = b.data();
  const size_t a_capacity = a.capacity();
  const size_t b_capacity = b.capacity();

  swap(a, b);

  assert(a.data() == b_data && b.data() == a_data);
  assert(a.capacity() == b_capacity && b.capacity() == a_capacity);
  assert(a.size() == 3 && b.size() == 2);
  assert(a[0] == 3 && a[1] == 4 && a[2] == 5);
  assert(b[0] == 1 && b[1] == 2);
}

inline void test_emplace_back() {
  Vector<std::pair<int, int>> v;
  v.reserve(2);

  auto& first = v.emplace_back(1, 2);
  assert(&first == &v[0]);
  assert(first.first == 1 && first.second == 2);

  v.emplace_back(3, 4);
  auto& third = v.emplace_back(5, 6);

  assert(&third == &v[2]);
  assert(v.size() == 3);
  assert(v[0].first == 1 && v[0].second == 2);
  assert(v[1].first == 3 && v[1].second == 4);
  assert(v[2].first == 5 && v[2].second == 6);
}

inline void test_move_assignment() {
  Vector<int> a{1, 2, 3};
  Vector<int> b{10, 20};

  b = std::move(a);

  assert(b.size() == 3);
  assert(b[0] == 1);
  assert(b[1] == 2);
  assert(b[2] == 3);

  assert(a.empty());
  assert(a.data() == nullptr);
}

inline void test_self_assignment() {
  Vector<int> v{1, 2, 3};

  v = v;

  assert(v.size() == 3);
  assert(v[0] == 1);
  assert(v[1] == 2);
  assert(v[2] == 3);
}

inline void test_push_back_aliasing() {
  Vector<int> v{1, 2, 3};

  v.shrink_to_fit();
  v.push_back(v[0]);

  assert(v.size() == 4);
  assert(v[0] == 1);
  assert(v[3] == 1);
}

inline void test_insert_aliasing() {
  Vector<int> v{1, 2, 3};

  v.insert(v.begin() + 1, 2, v[2]);

  assert(v.size() == 5);
  assert(v[0] == 1);
  assert(v[1] == 3);
  assert(v[2] == 3);
  assert(v[3] == 2);
  assert(v[4] == 3);
}

inline void test_basic() {
  test_constructor();
  test_initializer_list();
  test_push_back();
  test_insert();
  test_insert_without_reallocation();
  test_insert_zero();
  test_insert_rvalue();
  test_pop_back();
  test_resize();
  test_erase();
  test_clear();
  test_copy();
  test_move();
  test_at();
  test_reserve();
  test_shrink_to_fit();
  test_front_back();
  test_iterators();
  test_reverse_iterators();
  test_copy_assignment();
  test_copy_assignment_reuses_capacity();
  test_move_assignment();
  test_swap();
  test_emplace_back();
  test_self_assignment();
  test_push_back_aliasing();
  test_insert_aliasing();

  std::cout << "test_basic done\n";
}
