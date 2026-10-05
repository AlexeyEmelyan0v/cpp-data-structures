#include <cassert>

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
  test_move_assignment();
  test_self_assignment();
  test_push_back_aliasing();
  test_insert_aliasing();

  std::cout << "test_basic done\n";
}
