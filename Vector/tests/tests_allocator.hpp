#pragma once

#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "../vector.hpp"

namespace allocator_tests {

struct Allocation {
  int owner;
  std::size_t count;
};

inline std::unordered_map<void*, Allocation> allocations;
inline int allocator_swaps = 0;

template <typename T, bool Copy, bool Move, bool Swap, bool AlwaysEqual = false>
struct TrackingAllocator {
  using value_type = T;
  using propagate_on_container_copy_assignment = std::bool_constant<Copy>;
  using propagate_on_container_move_assignment = std::bool_constant<Move>;
  using propagate_on_container_swap = std::bool_constant<Swap>;
  using is_always_equal = std::bool_constant<AlwaysEqual>;

  template <typename U>
  struct rebind {
    using other = TrackingAllocator<U, Copy, Move, Swap, AlwaysEqual>;
  };

  int owner = 0;

  TrackingAllocator() = default;
  explicit TrackingAllocator(int id) noexcept: owner(id) {}

  template <typename U>
  TrackingAllocator(const TrackingAllocator<U, Copy, Move, Swap, AlwaysEqual>& other) noexcept: owner(other.owner) {}

  T* allocate(std::size_t count) {
    T* ptr = std::allocator<T>{}.allocate(count);
    allocations.emplace(ptr, Allocation{owner, count});
    return ptr;
  }

  void deallocate(T* ptr, std::size_t count) noexcept {
    auto it = allocations.find(ptr);
    assert(it != allocations.end());
    assert(AlwaysEqual || it->second.owner == owner);
    assert(it->second.count == count);
    allocations.erase(it);
    std::allocator<T>{}.deallocate(ptr, count);
  }

  TrackingAllocator select_on_container_copy_construction() const noexcept {
    return TrackingAllocator(owner + 100);
  }

  template <typename U>
  bool operator==(const TrackingAllocator<U, Copy, Move, Swap, AlwaysEqual>& other) const noexcept {
    return AlwaysEqual || owner == other.owner;
  }

  friend void swap(TrackingAllocator& lhs, TrackingAllocator& rhs) noexcept {
    ++allocator_swaps;
    std::swap(lhs.owner, rhs.owner);
  }
};

inline void test_copy_construction() {
  using A = TrackingAllocator<int, false, false, false>;
  {
    Vector<int, A> source(2, 7, A{1});
    Vector<int, A> selected(source);
    Vector<int, A> explicit_allocator(source, A{2});

    assert(selected.get_allocator().owner == 101);
    assert(explicit_allocator.get_allocator().owner == 2);
    assert(selected.size() == 2 && selected[0] == 7 && selected[1] == 7);
    assert(explicit_allocator.size() == 2 && explicit_allocator[0] == 7);
  }
  assert(allocations.empty());
}

inline void test_copy_assignment() {
  using Keep = TrackingAllocator<int, false, false, false>;
  {
    Vector<int, Keep> target(2, 1, Keep{1});
    Vector<int, Keep> source(3, 8, Keep{2});
    target = source;
    assert(target.get_allocator().owner == 1);
    assert(target.size() == 3 && target[0] == 8 && target[2] == 8);
  }
  assert(allocations.empty());

  using Propagate = TrackingAllocator<int, true, false, false>;
  {
    Vector<int, Propagate> target(2, 1, Propagate{3});
    Vector<int, Propagate> source(3, 9, Propagate{4});
    target = source;
    assert(target.get_allocator().owner == 4);
    assert(target.size() == 3 && target[0] == 9 && target[2] == 9);
  }
  assert(allocations.empty());
}

inline void test_move_construction() {
  using A = TrackingAllocator<int, false, false, false>;
  {
    Vector<int, A> source(2, 5, A{1});
    int* storage = source.data();
    Vector<int, A> equal(std::move(source), A{1});
    assert(equal.data() == storage);
    assert(source.empty());
    assert(equal.get_allocator().owner == 1);

    Vector<int, A> unequal(std::move(equal), A{2});
    assert(unequal.data() != storage);
    assert(unequal.get_allocator().owner == 2);
    assert(unequal.size() == 2 && unequal[0] == 5 && unequal[1] == 5);
    assert(equal.size() == 2);
  }
  assert(allocations.empty());
}

inline void test_move_assignment() {
  using Keep = TrackingAllocator<int, false, false, false>;
  {
    Vector<int, Keep> target(5, 1, Keep{1});
    Vector<int, Keep> source(3, 8, Keep{2});
    int* target_storage = target.data();
    int* source_storage = source.data();
    target = std::move(source);
    assert(target.get_allocator().owner == 1);
    assert(target.data() == target_storage);
    assert(source.data() == source_storage);
    assert(target.size() == 3 && target[0] == 8 && target[2] == 8);
    assert(source.size() == 3);
  }
  assert(allocations.empty());

  {
    Vector<int, Keep> target(1, 1, Keep{1});
    Vector<int, Keep> source(3, 8, Keep{2});
    int* source_storage = source.data();
    target = std::move(source);
    assert(target.get_allocator().owner == 1);
    assert(target.data() != source_storage);
    assert(target.size() == 3 && target[0] == 8 && target[2] == 8);
    assert(source.data() == source_storage && source.size() == 3);
  }
  assert(allocations.empty());

  using Propagate = TrackingAllocator<int, false, true, false>;
  {
    Vector<int, Propagate> target(2, 1, Propagate{3});
    Vector<int, Propagate> source(3, 9, Propagate{4});
    int* source_storage = source.data();
    target = std::move(source);
    assert(target.get_allocator().owner == 4);
    assert(target.data() == source_storage);
    assert(target.size() == 3 && target[0] == 9);
    assert(source.empty());
  }
  assert(allocations.empty());

  using AlwaysEqual = TrackingAllocator<int, false, false, false, true>;
  {
    Vector<int, AlwaysEqual> target(2, 1, AlwaysEqual{5});
    Vector<int, AlwaysEqual> source(3, 9, AlwaysEqual{6});
    int* source_storage = source.data();
    target = std::move(source);
    assert(target.get_allocator().owner == 5);
    assert(target.data() == source_storage);
    assert(target.size() == 3 && target[0] == 9);
    assert(source.empty());
  }
  assert(allocations.empty());
}

inline void test_swap() {
  using Keep = TrackingAllocator<int, false, false, false>;
  allocator_swaps = 0;
  {
    Vector<int, Keep> left(1, 3, Keep{1});
    Vector<int, Keep> right(2, 7, Keep{1});
    int* left_storage = left.data();
    left.swap(right);
    assert(left.size() == 2 && left[0] == 7);
    assert(right.size() == 1 && right[0] == 3);
    assert(right.data() == left_storage);
    assert(allocator_swaps == 0);
  }
  assert(allocations.empty());

  using Propagate = TrackingAllocator<int, false, false, true>;
  {
    Vector<int, Propagate> left(1, 3, Propagate{1});
    Vector<int, Propagate> right(2, 7, Propagate{2});
    left.swap(right);
    assert(left.get_allocator().owner == 2);
    assert(right.get_allocator().owner == 1);
    assert(left.size() == 2 && left[0] == 7);
    assert(right.size() == 1 && right[0] == 3);
    assert(allocator_swaps == 1);
  }
  assert(allocations.empty());
}

}  // namespace allocator_tests

inline void test_allocator() {
  allocator_tests::test_copy_construction();
  allocator_tests::test_copy_assignment();
  allocator_tests::test_move_construction();
  allocator_tests::test_move_assignment();
  allocator_tests::test_swap();
  std::cout << "tests_allocator done\n";
}
