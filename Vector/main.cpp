#include <iostream>

#include "tests/tests_basic.hpp"

int main() {
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

  std::cout << "All tests done\n";
}