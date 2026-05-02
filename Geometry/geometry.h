#include <iostream>
#include <vector>
#include <cmath>

const double eps = 1e-5;
const double pi = M_PI;

class Line;

struct Point {
  double x;
  double y;

  Point() : x(0), y(0) {};
  Point(double x, double y) : x(x), y(y) {};

  Point& operator+=(const Point&);
  Point& operator-=(const Point&);
  Point& operator*=(double);

  double len() const;

  double dist(const Point& point) const;

  bool isCollinearTo(const Point& point) const;

  void rotatePoint(const Point&, double);
  void rotateVector(const Point&, double);
  void reflectPoint(const Point&);
  void reflectVector(const Point&);
  void reflectPoint(const Line&);
  void reflectPoint(const Line&);
  void scale(const Point&, double);
};

bool operator==(const Point& point1, const Point& point2) {
  return std::abs(point1.x - point2.x) < eps && std::abs(point1.y - point2.y) < eps;
}

bool operator!=(const Point& point1, const Point& point2) {
  return !(point1 == point2);
}

Point& Point::operator+=(const Point& point) {
  x += point.x;
  y += point.y;
  return *this;
}

Point& Point::operator-=(const Point& point) {
  x -= point.x;
  y -= point.y;
  return *this;
}

Point& Point::operator*=(double k) {
  x *= k;
  y *= k;
  return *this;
}

Point operator+(const Point& point1, const Point& point2) {
  return {point1.x + point2.x, point1.y + point2.y};
}

Point operator-(const Point& point1, const Point& point2) {
  return {point1.x - point2.x, point1.y - point2.y};
}

Point operator*(const Point& point1, double coefficient) {
  return {point1.x * coefficient, point1.y * coefficient};
}

double Point::len() const {
  return sqrt(x * x + y * y);
}

double Point::dist(const Point& point) const {
  return (*this - point).len();
}

bool Point::isCollinearTo(const Point& point) const {
  return std::abs(x * point.y - y * point.x) < eps;
}

void Point::rotatePoint(const Point& center, double angle) {
  angle = angle * pi / 180;
  double x0 = x;
  double y0 = y;
  x = (x0 - center.x) * cos(angle) - (y0 - center.y) * sin(angle) + center.x;
  y = (x0 - center.x) * sin(angle) + (y0 - center.y) * cos(angle) + center.y;
}

void Point::rotateVector(const Point& center, double angle) {
  Point start(0, 0);
  Point end(x, y);
  start.rotatePoint(center, angle);
  end.rotatePoint(center, angle);
  x = end.x - start.x;
  y = end.y - start.y;
}

class Line {
 public:
  Point point_;
  Point vector_;
 public:
  Line(const Point& point1, const Point& point2) : point_(point1), vector_(point2 - point1) {};
  Line(double k, double b) : point_(0, b), vector_(1, k) {};
  Line(const Point& point, double k) : point_(point), vector_(1, k) {};

  bool operator==(const Line& line) const {
    return vector_.isCollinearTo(line.vector_) && vector_.isCollinearTo(point_ - line.point_);
  }

  bool operator!=(const Line& line) const {
    return !(*this == line);
  }

  void rotate(const Point&, double);
  void reflect(const Point&);
  void reflect(const Line&);
  void scale(const Point&, double);
};

void Line::rotate(const Point& center, double angle) {
  point_.rotatePoint(center, angle);
  vector_.rotateVector(center, angle);
}

class Shape {
 public:
  virtual double perimeter() const = 0;

  virtual double area() const = 0;

  virtual bool operator==(const Shape& another) const = 0;

  virtual bool isCongruentTo(const Shape& another) const = 0;

  virtual bool isSimilarTo(const Shape& another) const = 0;

  virtual bool containsPoint(const Point& point) const = 0;

  virtual void rotate(const Point& center, double angel) = 0;

  virtual void reflect(const Point& center) = 0;

  virtual void reflect(const Line& axis) = 0;

  virtual void scale(const Point& center, double coefficient) = 0;

  virtual ~Shape() = 0;
};

class Polygon : public Shape {
 protected:
  std::vector<Point> vertices_;
 public:
  Polygon() {};
  Polygon(const std::vector<Point>& vertices) : vertices_(vertices);

  template<typename... vertices>
  Polygon(vertices... args);

  int verticesCount() const;

  std::vector<Point> getVertices() const;

  bool isConvex() const;


};
