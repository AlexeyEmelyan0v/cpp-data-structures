#include <iostream>
#include <iomanip>
#include "geometry.h"

int main() {
  Point a(2, 3);
  Point v(3, 5);
  Line l(a, v);
  l.rotate({2, 2}, 60);
  a.rotatePoint({2, 2}, 60);
  std::cout << l.point_.x << ' ' << l.point_.y << '\n';
  std::cout << l.vector_.x << ' ' << l.vector_.y << '\n';
}