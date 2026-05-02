#include <iostream>
#include <type_traits>
#include <vector>
#include <compare>

template <typename T>
class Deque {
 private:
  static const size_t CHUNK_SIZE = 32;
  size_t front_chunk;
  size_t back_chunk;
  size_t front_ind;
  size_t back_ind;
  std::vector<T*> chunks;

  void ClearChunks();
  bool IsIndexWithinRange(size_t, size_t) const;
  void ReallocateChunks();
 public:
  Deque();
  Deque(const Deque&);
  Deque(size_t);
  Deque(size_t, const T&);
  Deque& operator=(Deque);

  ~Deque();

  size_t size() const;

  T& operator[](size_t);
  const T& operator[](size_t) const;
  T& at(size_t);
  const T& at(size_t) const;

  void push_back(const T&);
  void pop_back();
  void push_front(const T&);
  void pop_front();

  template <bool is_const>
  struct my_iterator {
    using value_type = T;
    using reference = std::conditional_t<is_const, const T&, T&>;
    using pointer = std::conditional_t<is_const, const T*, T*>;
    using iterator_category = std::random_access_iterator_tag;
    using deque_ptr = std::conditional_t<is_const, const Deque<T>*, Deque<T>*>;

    my_iterator() = default;
    my_iterator(const my_iterator& other) = default;
    my_iterator& operator=(const my_iterator& other) = default;
    my_iterator(int chunk_ind, int ind, deque_ptr deque) : chunk_ind(chunk_ind), ind(ind), deque(deque) {};

    my_iterator& operator++() {
      return (*this += 1);
    }

    my_iterator operator++(int) {
      my_iterator copy = *this;
      ++*this;
      return copy;
    }

    my_iterator& operator--() {
      return (*this -= 1);
    }

    my_iterator operator--(int) {
      my_iterator copy = *this;
      --*this;
      return copy;
    }

    my_iterator& operator+=(int offset) {
      chunk_ind += (ind + offset) / CHUNK_SIZE;
      ind = (ind + offset) % CHUNK_SIZE;
      return *this;
    }

    my_iterator& operator-=(int offset) {
      chunk_ind -= offset / CHUNK_SIZE;
      offset %= CHUNK_SIZE;
      if (ind < offset) {
        --chunk_ind;
        offset -= ind;
        ind = CHUNK_SIZE;
      }

      ind -= offset;
      return *this;
    }

    my_iterator operator+(int offset) const {
      my_iterator copy = *this;
      copy += offset;
      return copy;
    }

    my_iterator operator-(int offset) const {
      my_iterator copy = *this;
      copy -= offset;
      return copy;
    }

    bool operator<(const my_iterator& it) const {
      return ((chunk_ind < it.chunk_ind) || ((chunk_ind == it.chunk_ind) && ind < it.ind));
    }

    bool operator>(const my_iterator& it) const {
      return (it < *this);
    }

    bool operator>=(const my_iterator& it) {
      return !(*this < it);
    }

    bool operator<=(const my_iterator& it) {
      return !(it < *this);
    }

    bool operator==(const my_iterator& it) const {
      return (chunk_ind == it.chunk_ind) && (ind == it.ind);
    }

    bool operator!=(const my_iterator& it) const {
      return !(*this == it);
    }

    int operator-(const my_iterator& it) const {
      return ind + chunk_ind * CHUNK_SIZE - (it.ind + it.chunk_ind * CHUNK_SIZE);
    }

    operator my_iterator<true>() const {
      return my_iterator<true>(chunk_ind, ind, deque);
    }

    reference operator*() const {
      return deque->chunks[chunk_ind][ind];
    }

    pointer operator->() const {
      return &(deque->chunks[chunk_ind][ind]);
    }

   private:
    int chunk_ind;
    int ind;
    deque_ptr deque;
  };

  using size_type = size_t;
  using iterator = my_iterator<false>;
  using const_iterator = my_iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin();
  const_iterator begin() const;

  iterator end();
  const_iterator end() const;

  const_iterator cbegin() const;
  const_iterator cend() const;

  reverse_iterator rbegin();
  reverse_iterator rend();

  const_reverse_iterator crbegin() const;
  const_reverse_iterator crend() const;

  void insert(iterator it, const T& val);
  void erase(iterator it);
};

template <typename T>
void Deque<T>::ClearChunks() {
  for (size_t i = 0; i < chunks.size(); ++i) {
    delete[] reinterpret_cast<char*>(chunks[i]);
  }
}

template <typename T>
bool Deque<T>::IsIndexWithinRange(size_t i, size_t j) const {
  return (i > front_chunk && i < back_chunk) || (i == front_chunk && j > front_ind && i != back_chunk)
      || (i == back_chunk && j < back_ind && i != front_chunk)
      || (front_chunk == back_chunk && j > front_ind && j < back_ind);
}

template <typename T>
void Deque<T>::ReallocateChunks() {
  std::vector<T*> new_chunks(3 * chunks.size(), nullptr);

  for (size_t i = 0; i < chunks.size(); ++i) {
    new_chunks[i] = reinterpret_cast<T*>(new char[CHUNK_SIZE * sizeof(T)]);
    new_chunks[i + chunks.size()] = chunks[i];
    new_chunks[i + 2 * chunks.size()] = reinterpret_cast<T*>(new char[CHUNK_SIZE * sizeof(T)]);
  }

  front_chunk += chunks.size();
  back_chunk += chunks.size();
  chunks = new_chunks;
}

template <typename T>
Deque<T>::Deque(): front_chunk(0), back_chunk(1), front_ind(CHUNK_SIZE - 1), back_ind(0), chunks(2, nullptr) {
  chunks[0] = reinterpret_cast<T*>(new char[CHUNK_SIZE * sizeof(T)]);
  chunks[1] = reinterpret_cast<T*>(new char[CHUNK_SIZE * sizeof(T)]);
}

template <typename T>
Deque<T>::Deque(const Deque& other)
    : front_chunk(other.front_chunk),
      back_chunk(other.back_chunk),
      front_ind(other.front_ind),
      back_ind(other.back_ind),
      chunks(other.chunks.size(), nullptr) {
  for (size_t i = 0; i < chunks.size(); ++i) {
    chunks[i] = reinterpret_cast<T*>(new char[CHUNK_SIZE * sizeof(T)]);
  }

  size_t i;
  size_t j;

  try {
    for (i = front_chunk; i <= back_chunk; ++i) {
      for (j = 0; j < CHUNK_SIZE; ++j) {
        if (IsIndexWithinRange(i, j)) {
          new(chunks[i] + j) T(other.chunks[i][j]);
        }
      }
    }
  } catch (...) {
    for (size_t i1 = front_chunk; i1 <= std::min(i, back_chunk); ++i1) {
      for (size_t j1 = 0; j1 < CHUNK_SIZE; ++j1) {
        if (IsIndexWithinRange(i1, j1) && ((i1 == i && j1 < j) || (i1 < i))) {
          (chunks[i1] + j1)->~T();
        }
      }
    }

    ClearChunks();
    throw;
  }
}

template <typename T>
Deque<T>::Deque(size_t sz)
    : front_chunk(0),
      back_chunk(sz / CHUNK_SIZE + 1),
      front_ind(CHUNK_SIZE - 1),
      back_ind(0),
      chunks(sz / CHUNK_SIZE + 2, nullptr) {
  for (size_t i = 0; i < chunks.size(); i++) {
    chunks[i] = reinterpret_cast<T*>(new char[CHUNK_SIZE * sizeof(T)]);
  }

  size_t i;
  size_t j;

  try {
    for (i = front_chunk + 1; i < back_chunk; ++i) {
      for (j = 0; j < CHUNK_SIZE; ++j) {
        if constexpr (std::is_default_constructible_v<T>) {
          new(chunks[i] + j) T();
        }
      }
    }
  } catch (...) {
    for (size_t i1 = front_chunk + 1; i1 <= i; ++i1) {
      for (size_t j1 = 0; j1 < CHUNK_SIZE; ++j1) {
        if ((i1 == i && j1 < j) || i1 < i) {
          (chunks[i1] + j1)->~T();
        }
      }
    }

    ClearChunks();
    throw;
  }

  try {
    for (j = 0; j < sz % CHUNK_SIZE; ++j) {
      if constexpr (std::is_constructible_v<T>) {
        new(chunks[back_chunk] + j) T();
        ++back_ind;
      }
    }
  } catch (...) {
    for (size_t j1 = 0; j1 < j; ++j1) {
      (chunks[back_chunk] + j1)->~T();
    }

    for (size_t i1 = front_chunk + 1; i1 < back_chunk; ++i1) {
      for (size_t j1 = 0; j1 < CHUNK_SIZE; ++j1) {
        (chunks[i1] + j1)->~T();
      }
    }

    ClearChunks();
    throw;
  }
}

template <typename T>
Deque<T>::Deque(size_t sz, const T& value):
    front_chunk(0),
    back_chunk(sz / CHUNK_SIZE + 1),
    front_ind(CHUNK_SIZE - 1),
    back_ind(0),
    chunks(sz / CHUNK_SIZE + 2, nullptr) {
  for (size_t i = 0; i < chunks.size(); i++) {
    chunks[i] = reinterpret_cast<T*>(new char[CHUNK_SIZE * sizeof(T)]);
  }

  size_t i;
  size_t j;

  try {
    for (i = front_chunk + 1; i < back_chunk; ++i) {
      for (j = 0; j < CHUNK_SIZE; ++j) {
        new(chunks[i] + j) T(value);
      }
    }
  } catch (...) {
    for (size_t i1 = front_chunk + 1; i1 <= i; ++i1) {
      for (size_t j1 = 0; j1 < CHUNK_SIZE; ++j1) {
        if ((i1 == i && j1 < j) || i1 < i) {
          (chunks[i1] + j1)->~T();
        }
      }
    }

    ClearChunks();
    throw;
  }

  try {
    for (j = 0; j < sz % CHUNK_SIZE; ++j) {
      new(chunks[back_chunk] + j) T(value);
      ++back_ind;
    }
  } catch (...) {
    for (size_t j1 = 0; j1 < j; ++j1) {
      (chunks[back_chunk] + j1)->~T();
    }

    for (size_t i1 = front_chunk + 1; i1 < back_chunk; ++i1) {
      for (size_t j1 = 0; j1 < CHUNK_SIZE; ++j1) {
        (chunks[i1] + j1)->~T();
      }
    }

    ClearChunks();
    throw;
  }
}

template <typename T>
Deque<T>& Deque<T>::operator=(Deque<T> other) {
  std::swap(front_chunk, other.front_chunk);
  std::swap(front_ind, other.front_ind);
  std::swap(back_chunk, other.back_chunk);
  std::swap(back_ind, other.back_ind);
  std::swap(chunks, other.chunks);
  return *this;
}

template <typename T>
Deque<T>::~Deque() {
  for (size_t i = front_chunk + 1; i < back_chunk; ++i) {
    for (size_t j = 0; j < CHUNK_SIZE; ++j) {
      (chunks[i] + j)->~T();
    }
  }

  if (front_chunk == back_chunk) {
    for (size_t j = front_ind + 1; j < back_ind; ++j) {
      (chunks[front_chunk] + j)->~T();
    }
  } else {
    for (size_t j = front_ind + 1; j < CHUNK_SIZE; ++j) {
      (chunks[front_chunk] + j)->~T();
    }

    for (size_t j = 0; j < back_ind; ++j) {
      (chunks[back_chunk] + j)->~T();
    }
  }

  ClearChunks();
}

template <typename T>
size_t Deque<T>::size() const {
  if (front_chunk == back_chunk) {
    return back_ind - front_ind - 1;
  }

  return (back_chunk - front_chunk - 1) * CHUNK_SIZE + back_ind + (CHUNK_SIZE - 1 - front_ind);
}

template <typename T>
T& Deque<T>::operator[](size_t ind) {
  if ((front_chunk == back_chunk) || (ind + front_ind < CHUNK_SIZE - 1)) {
    return chunks[front_chunk][front_ind + ind + 1];
  }

  return chunks[front_chunk + (ind - (CHUNK_SIZE - 1 - front_ind)) / CHUNK_SIZE + 1][
      (ind - (CHUNK_SIZE - 1 - front_ind)) % CHUNK_SIZE];
}

template <typename T>
const T& Deque<T>::operator[](size_t ind) const {
  if ((front_chunk == back_chunk) || (ind + front_ind < CHUNK_SIZE - 1)) {
    return chunks[front_chunk][front_ind + ind + 1];
  }

  return chunks[front_chunk + (ind - (CHUNK_SIZE - 1 - front_ind)) / CHUNK_SIZE + 1][
      (ind - (CHUNK_SIZE - 1 - front_ind)) % CHUNK_SIZE];
}

template <typename T>
T& Deque<T>::at(size_t ind) {
  if (ind >= size()) {
    throw std::out_of_range("Index out of range!");
  }

  return (*this)[ind];
}

template <typename T>
const T& Deque<T>::at(size_t ind) const {
  if (ind >= size()) {
    throw std::out_of_range("Index out of range!");
  }

  return *this[ind];
}

template <typename T>
void Deque<T>::push_back(const T& value) {
  try {
    new(chunks[back_chunk] + back_ind) T(value);
    ++back_ind;
  } catch (...) {
    throw;
  }

  if (back_ind == CHUNK_SIZE) {
    ++back_chunk;
    back_ind = 0;

    if (back_chunk == chunks.size()) {
      ReallocateChunks();
    }
  }
}

template <typename T>
void Deque<T>::push_front(const T& value) {
  try {
    new(chunks[front_chunk] + front_ind) T(value);
  } catch (...) {
    throw;
  }

  if (front_ind == 0) {
    if (front_chunk == 0) {
      ReallocateChunks();
    }

    front_ind = CHUNK_SIZE - 1;
    --front_chunk;
  } else {
    --front_ind;
  }
}

template <typename T>
void Deque<T>::pop_back() {
  if (back_ind == 0) {
    --back_chunk;
    back_ind = CHUNK_SIZE;
  }

  --back_ind;
  (chunks[back_chunk] + back_ind)->~T();
}

template <typename T>
void Deque<T>::pop_front() {
  if (front_ind == CHUNK_SIZE - 1) {
    ++front_chunk;
    front_ind = 0;
  } else {
    ++front_ind;
  }

  (chunks[front_chunk] + front_ind)->~T();
}

template <typename T>
typename Deque<T>::iterator Deque<T>::begin() {
  if (front_ind == CHUNK_SIZE - 1) {
    return iterator(front_chunk + 1, 0, this);
  }

  return iterator(front_chunk, front_ind + 1, this);
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::begin() const {
  if (front_ind == CHUNK_SIZE - 1) {
    return const_iterator(front_chunk + 1, 0, this);
  }

  return const_iterator(front_chunk, front_ind + 1, this);
}

template <typename T>
typename Deque<T>::iterator Deque<T>::end() {
  return iterator(back_chunk, back_ind, this);
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::end() const {
  return const_iterator(back_chunk, back_ind, this);
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::cbegin() const {
  if (front_ind == CHUNK_SIZE - 1) {
    return const_iterator(front_chunk + 1, 0, this);
  }

  return const_iterator(front_chunk, front_ind + 1, this);
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::cend() const {
  return const_iterator(back_chunk, back_ind, this);
}

template <typename T>
typename Deque<T>::reverse_iterator Deque<T>::rbegin() {
  return reverse_iterator(end());
}

template <typename T>
typename Deque<T>::reverse_iterator Deque<T>::rend() {
  return reverse_iterator(begin());
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::crbegin() const {
  return const_reverse_iterator(cend());
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::crend() const {
  return const_reverse_iterator(cbegin());
}

template <typename T>
void Deque<T>::insert(iterator it, const T& value) {
  if (it == end()) {
    push_back(value);
  } else {
    insert(it + 1, *it);
    it->~T();
    new(&(*it)) T(value);
  }
}

template <typename T>
void Deque<T>::erase(iterator it) {
  if (it == (end() - 1)) {
    pop_back();
  } else {
    it->~T();
    new(&(*it)) T(*(it + 1));
    erase(it + 1);
  }
}
