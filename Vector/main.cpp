#include <iostream>

#include "tests/tests_allocator.hpp"
#include "tests/tests_basic.hpp"
#include "tests/tests_edge.hpp"
#include "tests/tests_exception.hpp"
#include "tests/tests_random.hpp"

int main() {
  test_basic();
  test_exception();
  test_allocator();
  test_random();
  test_edge();
  std::cout << "\nAll tests done!\n";
}
