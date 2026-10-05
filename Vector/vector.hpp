#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <utility>

template <typename T>
class Vector {
 private:
  T* data_;
  size_t size_;
  size_t capacity_;

  static void destroy_elements(T* data, size_t count) noexcept;

  template <typename U>
  void reallocate_and_push(U&& value);

  template <typename... Args>
  T& reallocate_and_emplace(Args&&... args);

 public:
  Vector();
  Vector(const Vector& other);
  Vector(Vector&& other) noexcept;

  explicit Vector(size_t count);
  Vector(size_t count, const T& value);
  Vector(std::initializer_list<T> init);

  ~Vector();

  Vector& operator=(const Vector& other);
  Vector& operator=(Vector&& other) noexcept;

  void reserve(size_t newcap);
  void push_back(const T& value);
  void push_back(T&& value);

  size_t size() const noexcept;
  size_t capacity() const noexcept;
  T* data() noexcept;
  const T* data() const noexcept;

  bool empty() const noexcept;
  void swap(Vector& other) noexcept;
  void clear() noexcept;
  void pop_back() noexcept;
  void resize(size_t newsize);
  void shrink_to_fit();

  T& front() noexcept;
  const T& front() const noexcept;
  T& back() noexcept;
  const T& back() const noexcept;

  T& operator[](size_t index) noexcept;
  const T& operator[](size_t index) const noexcept;

  T& at(size_t index);
  const T& at(size_t index) const;

  template <typename... Args>
  T& emplace_back(Args&&... args);

  using size_type = size_t;
  using iterator = T*;
  using const_iterator = const T*;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin() noexcept;
  iterator end() noexcept;

  const_iterator begin() const noexcept;
  const_iterator end() const noexcept;

  const_iterator cbegin() const noexcept;
  const_iterator cend() const noexcept;

  reverse_iterator rbegin() noexcept;
  reverse_iterator rend() noexcept;

  const_reverse_iterator rbegin() const noexcept;
  const_reverse_iterator rend() const noexcept;

  const_reverse_iterator crbegin() const noexcept;
  const_reverse_iterator crend() const noexcept;

  iterator insert(const_iterator pos, const T& value);
  // iterator insert(const_iterator pos, T&& value);
  iterator insert(const_iterator pos, size_t count, const T& value);

  iterator erase(const_iterator pos);
  iterator erase(const_iterator first, const_iterator last);
};

#include "vector.tpp"