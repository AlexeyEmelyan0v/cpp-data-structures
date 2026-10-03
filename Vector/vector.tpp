template <typename T>
void Vector<T>::destroy_elements(T* data, size_t count) noexcept {
  for (size_t i = 0; i < count; ++i) {
    (data + i)->~T();
  }
}

template <typename T>
template <typename U>
void Vector<T>::reallocate_and_push(U&& value) {
  size_t newcap = (capacity_ > 0 ? capacity_ * 2 : 1);
  T* newdata = reinterpret_cast<T*>(new char[newcap * sizeof(T)]);
  size_t index = 0;
  bool new_element_constructed = false;

  try {
    new(newdata + size_) T(std::forward<U>(value));
    new_element_constructed = true;

    for (; index < size_; ++index) {
      new(newdata + index) T(std::move_if_noexcept(data_[index]));
    }
  } catch (...) {
    destroy_elements(newdata, index);

    if (new_element_constructed) {
      (newdata + size_)->~T();
    }

    delete[] reinterpret_cast<char*>(newdata);
    throw;
  }

  destroy_elements(data_, size_);
  delete[] reinterpret_cast<char*>(data_);

  data_ = newdata;
  capacity_ = newcap;
  ++size_;
}

template <typename T>
template <typename... Args>
T& Vector<T>::reallocate_and_emplace(Args&&... args) {
  size_t newcap = (capacity_ > 0 ? capacity_ * 2 : 1);
  T* newdata = reinterpret_cast<T*>(new char[newcap * sizeof(T)]);
  size_t index = 0;
  bool new_element_constructed = false;

  try {
    new(newdata + size_) T(std::forward<Args>(args)...);
    new_element_constructed = true;

    for (; index < size_; ++index) {
      new(newdata + index) T(std::move_if_noexcept(data_[index]));
    }
  } catch (...) {
    destroy_elements(newdata, index);

    if (new_element_constructed) {
      (newdata + size_)->~T();
    }

    delete[] reinterpret_cast<char*>(newdata);
    throw;
  }

  destroy_elements(data_, size_);
  delete[] reinterpret_cast<char*>(data_);

  data_ = newdata;
  capacity_ = newcap;
  ++size_;
  return data_[size_ - 1];
}

template <typename T>
Vector<T>::Vector(): data_(nullptr), size_(0), capacity_(0) {}

template <typename T>
Vector<T>::Vector(const Vector& other): data_(nullptr), size_(0), capacity_(0) {
  reserve(other.size_);

  try {
    for (; size_ < other.size_; ++size_) {
      new(data_ + size_) T(other.data_[size_]);
    }
  } catch (...) {
    destroy_elements(data_, size_);
    delete[] reinterpret_cast<char*>(data_);
    throw;
  }
}

template <typename T>
Vector<T>::Vector(Vector&& other) noexcept
    : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
  other.data_ = nullptr;
  other.size_ = 0;
  other.capacity_ = 0;
}

template <typename T>
Vector<T>::Vector(size_t count): data_(nullptr), size_(0), capacity_(0) {
  reserve(count);

  try {
    for (; size_ < count; ++size_) {
      new(data_ + size_) T();
    }
  } catch (...) {
    destroy_elements(data_, size_);
    delete[] reinterpret_cast<char*>(data_);
    throw;
  }
}

template <typename T>
Vector<T>::Vector(size_t count, const T& value): data_(nullptr), size_(0), capacity_(0) {
  reserve(count);

  try {
    for (; size_ < count; ++size_) {
      new(data_ + size_) T(value);
    }
  } catch (...) {
    destroy_elements(data_, size_);
    delete[] reinterpret_cast<char*>(data_);
    throw;
  }
}

template <typename T>
Vector<T>::Vector(std::initializer_list<T> init): data_(nullptr), size_(0), capacity_(0) {
  reserve(init.size());

  try {
    for (const T& value : init) {
      new(data_ + size_) T(value);
      ++size_;
    }
  } catch (...) {
    destroy_elements(data_, size_);
    delete[] reinterpret_cast<char*>(data_);
    throw;
  }
}

template <typename T>
Vector<T>::~Vector() {
  destroy_elements(data_, size_);
  delete[] reinterpret_cast<char*>(data_);
}

template <typename T>
Vector<T>& Vector<T>::operator=(Vector other) {
  swap(other);
  return *this;
}

template <typename T>
Vector<T>& Vector<T>::operator=(Vector&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  destroy_elements(data_, size_);
  delete[] reinterpret_cast<char*>(data_);

  data_ = other.data_;
  size_ = other.size_;
  capacity_ = other.capacity_;

  other.data_ = nullptr;
  other.size_ = 0;
  other.capacity_ = 0;

  return *this;
}

template <typename T>
void Vector<T>::reserve(size_t newcap) {
  if (newcap <= capacity_) {
    return;
  }

  T* newdata = reinterpret_cast<T*>(new char[newcap * sizeof(T)]);
  size_t index = 0;

  try {
    for (; index < size_; ++index) {
      new(newdata + index) T(std::move_if_noexcept(data_[index]));
    }
  } catch (...) {
    destroy_elements(newdata, index);
    delete[] reinterpret_cast<char*>(newdata);
    throw;
  }

  destroy_elements(data_, size_);
  delete[] reinterpret_cast<char*>(data_);
  data_ = newdata;
  capacity_ = newcap;
}

template <typename T>
void Vector<T>::push_back(const T& value) {
  if (size_ < capacity_) {
    new(data_ + size_) T(value);
    ++size_;
    return;
  }

  reallocate_and_push(value);
}

template <typename T>
void Vector<T>::push_back(T&& value) {
  if (size_ < capacity_) {
    new(data_ + size_) T(std::move(value));
    ++size_;
    return;
  }

  reallocate_and_push(std::move(value));
}

template <typename T>
size_t Vector<T>::size() const noexcept {
  return size_;
}

template <typename T>
size_t Vector<T>::capacity() const noexcept {
  return capacity_;
}

template <typename T>
T* Vector<T>::data() noexcept {
  return data_;
}

template <typename T>
const T* Vector<T>::data() const noexcept {
  return data_;
}

template <typename T>
bool Vector<T>::empty() const noexcept {
  return size_ == 0;
}

template <typename T>
void Vector<T>::swap(Vector& other) noexcept {
  std::swap(data_, other.data_);
  std::swap(size_, other.size_);
  std::swap(capacity_, other.capacity_);
}

template <typename T>
void Vector<T>::clear() noexcept {
  destroy_elements(data_, size_);
  size_ = 0;
}

template <typename T>
void Vector<T>::pop_back() noexcept {
  --size_;
  (data_ + size_)->~T();
}

template <typename T>
void Vector<T>::resize(size_t newsize) {
  if (newsize == size_) {
    return;
  }

  if (newsize < size_) {
    destroy_elements(data_ + newsize, size_ - newsize);
    size_ = newsize;
    return;
  }

  reserve(newsize);
  size_t index = size_;

  try {
    for (; index < newsize; ++index) {
      new(data_ + index) T();
    }
  } catch (...) {
    destroy_elements(data_ + size_, index - size_);
    throw;
  }

  size_ = newsize;
}

template <typename T>
void Vector<T>::shrink_to_fit() {
  if (capacity_ == size_) {
    return;
  }

  if (size_ == 0) {
    delete[] reinterpret_cast<char*>(data_);
    data_ = nullptr;
    capacity_ = 0;
    return;
  }

  T* newdata = reinterpret_cast<T*>(new char[size_ * sizeof(T)]);
  size_t index = 0;

  try {
    for (; index < size_; ++index) {
      new(newdata + index) T(std::move_if_noexcept(data_[index]));
    }
  } catch (...) {
    destroy_elements(newdata, index);
    delete[] reinterpret_cast<char*>(newdata);
    throw;
  }

  destroy_elements(data_, size_);
  delete[] reinterpret_cast<char*>(data_);
  capacity_ = size_;
  data_ = newdata;
}

template <typename T>
T& Vector<T>::front() noexcept {
  return *data_;
}

template <typename T>
const T& Vector<T>::front() const noexcept {
  return *data_;
}

template <typename T>
T& Vector<T>::back() noexcept {
  return data_[size_ - 1];
}

template <typename T>
const T& Vector<T>::back() const noexcept {
  return data_[size_ - 1];
}

template <typename T>
T& Vector<T>::operator[](size_t index) noexcept {
  return data_[index];
}

template <typename T>
const T& Vector<T>::operator[](size_t index) const noexcept {
  return data_[index];
}

template <typename T>
T& Vector<T>::at(size_t index) {
  if (index >= size_) {
    throw std::out_of_range("Index is out of range");
  }

  return data_[index];
}

template <typename T>
const T& Vector<T>::at(size_t index) const {
  if (index >= size_) {
    throw std::out_of_range("Index is out of range");
  }

  return data_[index];
}

template <typename T>
template <typename... Args>
T& Vector<T>::emplace_back(Args&&... args) {
  if (size_ < capacity_) {
    new(data_ + size_) T(std::forward<Args>(args)...);
    ++size_;
    return data_[size_ - 1];
  }

  return reallocate_and_emplace(std::forward<Args>(args)...);
}

template <typename T>
typename Vector<T>::iterator Vector<T>::begin() noexcept {
  return data_;
}

template <typename T>
typename Vector<T>::iterator Vector<T>::end() noexcept {
  return data_ + size_;
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::begin() const noexcept {
  return data_;
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::end() const noexcept {
  return data_ + size_;
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::cbegin() const noexcept {
  return data_;
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::cend() const noexcept {
  return data_ + size_;
}

template <typename T>
typename Vector<T>::reverse_iterator Vector<T>::rbegin() noexcept {
  return reverse_iterator(end());
}

template <typename T>
typename Vector<T>::reverse_iterator Vector<T>::rend() noexcept {
  return reverse_iterator(begin());
}

template <typename T>
typename Vector<T>::const_reverse_iterator Vector<T>::rbegin() const noexcept {
  return const_reverse_iterator(end());
}

template <typename T>
typename Vector<T>::const_reverse_iterator Vector<T>::rend() const noexcept {
  return const_reverse_iterator(begin());
}

template <typename T>
typename Vector<T>::const_reverse_iterator Vector<T>::crbegin() const noexcept {
  return const_reverse_iterator(cend());
}

template <typename T>
typename Vector<T>::const_reverse_iterator Vector<T>::crend() const noexcept {
  return const_reverse_iterator(cbegin());
}

template <typename T>
typename Vector<T>::iterator Vector<T>::insert(const_iterator pos, size_t count, const T& value) {
  size_t pos_ind = pos - cbegin();

  if (count == 0) {
    return data_ + pos_ind;
  }

  /*
  if (capacity_ >= size_ + count) {
    T value_copy = value;
  TODO
  }
  */

  T* newdata = reinterpret_cast<T*>(new char[(size_ + count) * sizeof(T)]);
  size_t count_index = 0;
  size_t index = 0;

  try {
    for (; count_index < count; ++count_index) {
      new(newdata + pos_ind + count_index) T(value);
    }

    for (; index < pos_ind; ++index) {
      new(newdata + index) T(std::move_if_noexcept(data_[index]));
    }

    for (; index < size_; ++index) {
      new(newdata + count + index) T(std::move_if_noexcept(data_[index]));
    }
  } catch (...) {
    destroy_elements(newdata + pos_ind, count_index);

    if (index < pos_ind) {
      destroy_elements(newdata, index);
    } else {
      destroy_elements(newdata, pos_ind);
      destroy_elements(newdata + pos_ind + count, index - pos_ind);
    }

    delete[] reinterpret_cast<char*>(newdata);
    throw;
  }

  destroy_elements(data_, size_);
  delete[] reinterpret_cast<char*>(data_);
  data_ = newdata;
  size_ += count;
  capacity_ = size_;
  return data_ + pos_ind;
}

template <typename T>
typename Vector<T>::iterator Vector<T>::insert(const_iterator pos, const T& value) {
  return insert(pos, 1, value);
}

template <typename T>
typename Vector<T>::iterator Vector<T>::erase(const_iterator first, const_iterator last) {
  size_t count = last - first;
  size_t first_ind = first - cbegin();

  if (count == 0) {
    return data_ + first_ind;
  }

  for (size_t i = first_ind; i + count < size_; ++i) {
    data_[i] = std::move(data_[i + count]);
  }

  destroy_elements(data_ + size_ - count, count);
  size_ -= count;
  return data_ + first_ind;
}

template <typename T>
typename Vector<T>::iterator Vector<T>::erase(const_iterator pos) {
  return erase(pos, pos + 1);
}
