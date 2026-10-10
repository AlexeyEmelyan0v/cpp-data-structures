template <typename T, typename Allocator>
void Vector<T, Allocator>::destroy_elements(T* data, size_t count) noexcept {
  for (size_t i = 0; i < count; ++i) {
    AllocTraits::destroy(allocator_, data + i);
  }
}

template <typename T, typename Allocator>
template <typename U>
void Vector<T, Allocator>::reallocate_and_push(U&& value) {
  size_t newcap = (capacity_ > 0 ? capacity_ * 2 : 1);
  T* newdata = AllocTraits::allocate(allocator_, newcap);
  size_t index = 0;
  bool new_element_constructed = false;

  try {
    AllocTraits::construct(allocator_, newdata + size_, std::forward<U>(value));
    new_element_constructed = true;

    for (; index < size_; ++index) {
      AllocTraits::construct(allocator_, newdata + index, std::move_if_noexcept(data_[index]));
    }
  } catch (...) {
    destroy_elements(newdata, index);

    if (new_element_constructed) {
      AllocTraits::destroy(allocator_, newdata + size_);
    }

    AllocTraits::deallocate(allocator_, newdata, newcap);
    throw;
  }

  destroy_elements(data_, size_);

  if (data_ != nullptr) {
    AllocTraits::deallocate(allocator_, data_, capacity_);
  }

  data_ = newdata;
  capacity_ = newcap;
  ++size_;
}

template <typename T, typename Allocator>
template <typename... Args>
T& Vector<T, Allocator>::reallocate_and_emplace(Args&&... args) {
  size_t newcap = (capacity_ > 0 ? capacity_ * 2 : 1);
  T* newdata = AllocTraits::allocate(allocator_, newcap);
  size_t index = 0;
  bool new_element_constructed = false;

  try {
    AllocTraits::construct(allocator_, newdata + size_, std::forward<Args>(args)...);
    new_element_constructed = true;

    for (; index < size_; ++index) {
      AllocTraits::construct(allocator_, newdata + index, std::move_if_noexcept(data_[index]));
    }
  } catch (...) {
    destroy_elements(newdata, index);

    if (new_element_constructed) {
      AllocTraits::destroy(allocator_, newdata + size_);
    }

    AllocTraits::deallocate(allocator_, newdata, newcap);
    throw;
  }

  destroy_elements(data_, size_);

  if (data_ != nullptr) {
    AllocTraits::deallocate(allocator_, data_, capacity_);
  }

  data_ = newdata;
  capacity_ = newcap;
  ++size_;
  return data_[size_ - 1];
}

template <typename T, typename Allocator>
Vector<T, Allocator>::Vector() noexcept(std::is_nothrow_default_constructible_v<Allocator>)
    : data_(nullptr), size_(0), capacity_(0), allocator_() {}

template <typename T, typename Allocator>
Vector<T, Allocator>::Vector(const Allocator& allocator) noexcept
    : data_(nullptr), size_(0), capacity_(0), allocator_(allocator) {}

template <typename T, typename Allocator>
Vector<T, Allocator>::Vector(const Vector& other)
    : Vector(other, AllocTraits::select_on_container_copy_construction(other.allocator_)) {}

template <typename T, typename Allocator>
Vector<T, Allocator>::Vector(const Vector& other, const Allocator& allocator)
    : data_(nullptr), size_(0), capacity_(0), allocator_(allocator) {
  reserve(other.size_);

  try {
    for (; size_ < other.size_; ++size_) {
      AllocTraits::construct(allocator_, data_ + size_, other.data_[size_]);
    }
  } catch (...) {
    destroy_elements(data_, size_);

    if (data_ != nullptr) {
      AllocTraits::deallocate(allocator_, data_, capacity_);
    }

    throw;
  }
}

template <typename T, typename Allocator>
Vector<T, Allocator>::Vector(Vector&& other) noexcept
    : data_(other.data_), size_(other.size_), capacity_(other.capacity_), allocator_(std::move(other.allocator_)) {
  other.data_ = nullptr;
  other.size_ = 0;
  other.capacity_ = 0;
}

template <typename T, typename Allocator>
Vector<T, Allocator>::Vector(Vector&& other, const Allocator& allocator)
    : data_(nullptr), size_(0), capacity_(0), allocator_(allocator) {
  if (allocator == other.allocator_) {
    data_ = other.data_;
    size_ = other.size_;
    capacity_ = other.capacity_;

    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
    return;
  }

  reserve(other.size_);

  try {
    for (; size_ < other.size_; ++size_) {
      AllocTraits::construct(allocator_, data_ + size_, std::move(other.data_[size_]));
    }
  } catch (...) {
    destroy_elements(data_, size_);

    if (data_ != nullptr) {
      AllocTraits::deallocate(allocator_, data_, capacity_);
    }

    throw;
  }
}

template <typename T, typename Allocator>
Vector<T, Allocator>::Vector(size_t count, const Allocator& allocator)
    : data_(nullptr), size_(0), capacity_(0), allocator_(allocator) {
  reserve(count);

  try {
    for (; size_ < count; ++size_) {
      AllocTraits::construct(allocator_, data_ + size_);
    }
  } catch (...) {
    destroy_elements(data_, size_);

    if (data_ != nullptr) {
      AllocTraits::deallocate(allocator_, data_, capacity_);
    }

    throw;
  }
}

template <typename T, typename Allocator>
Vector<T, Allocator>::Vector(size_t count, const T& value, const Allocator& allocator)
    : data_(nullptr), size_(0), capacity_(0), allocator_(allocator) {
  reserve(count);

  try {
    for (; size_ < count; ++size_) {
      AllocTraits::construct(allocator_, data_ + size_, value);
    }
  } catch (...) {
    destroy_elements(data_, size_);

    if (data_ != nullptr) {
      AllocTraits::deallocate(allocator_, data_, capacity_);
    }

    throw;
  }
}

template <typename T, typename Allocator>
Vector<T, Allocator>::Vector(std::initializer_list<T> init, const Allocator& allocator)
    : data_(nullptr), size_(0), capacity_(0), allocator_(allocator) {
  reserve(init.size());

  try {
    for (const T& value : init) {
      AllocTraits::construct(allocator_, data_ + size_, value);
      ++size_;
    }
  } catch (...) {
    destroy_elements(data_, size_);

    if (data_ != nullptr) {
      AllocTraits::deallocate(allocator_, data_, capacity_);
    }

    throw;
  }
}

template <typename T, typename Allocator>
Vector<T, Allocator>::~Vector() {
  destroy_elements(data_, size_);

  if (data_ != nullptr) {
    AllocTraits::deallocate(allocator_, data_, capacity_);
  }
}

template <typename T, typename Allocator>
Vector<T, Allocator>& Vector<T, Allocator>::operator=(const Vector& other) {
  if (this == &other) {
    return *this;
  }

  if constexpr (AllocTraits::propagate_on_container_copy_assignment::value) {
    if (allocator_ != other.allocator_) {
      destroy_elements(data_, size_);

      if (data_ != nullptr) {
        AllocTraits::deallocate(allocator_, data_, capacity_);
      }

      data_ = nullptr;
      size_ = 0;
      capacity_ = 0;
    }

    allocator_ = other.allocator_;
  }

  if (other.size_ > capacity_) {
    Vector temp(other, allocator_);
    std::swap(data_, temp.data_);
    std::swap(size_, temp.size_);
    std::swap(capacity_, temp.capacity_);
    return *this;
  }

  size_t common_size = std::min(size_, other.size_);

  for (size_t i = 0; i < common_size; ++i) {
    data_[i] = other.data_[i];
  }

  if (size_ > other.size_) {
    destroy_elements(data_ + other.size_, size_ - other.size_);
    size_ = other.size_;
  } else {
    size_t old_size = size_;

    try {
      for (; size_ < other.size_; ++size_) {
        AllocTraits::construct(allocator_, data_ + size_, other.data_[size_]);
      }
    } catch (...) {
      destroy_elements(data_ + old_size, size_ - old_size);
      size_ = old_size;
      throw;
    }
  }

  return *this;
}

template <typename T, typename Allocator>
Vector<T, Allocator>& Vector<T, Allocator>::operator=(Vector&& other) noexcept(
    AllocTraits::propagate_on_container_move_assignment::value || AllocTraits::is_always_equal::value) {
  if (this == &other) {
    return *this;
  }

  constexpr bool propagate = AllocTraits::propagate_on_container_move_assignment::value;

  constexpr bool always_equal = AllocTraits::is_always_equal::value;

  if constexpr (propagate || always_equal) {
    destroy_elements(data_, size_);

    if (data_ != nullptr) {
      AllocTraits::deallocate(allocator_, data_, capacity_);
    }

    if constexpr (propagate) {
      allocator_ = std::move(other.allocator_);
    }

    data_ = other.data_;
    size_ = other.size_;
    capacity_ = other.capacity_;

    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
  } else {
    if (allocator_ == other.allocator_) {
      destroy_elements(data_, size_);

      if (data_ != nullptr) {
        AllocTraits::deallocate(allocator_, data_, capacity_);
      }

      data_ = other.data_;
      size_ = other.size_;
      capacity_ = other.capacity_;

      other.data_ = nullptr;
      other.size_ = 0;
      other.capacity_ = 0;
    } else {
      if (other.size_ > capacity_) {
        Vector temp(std::move(other), allocator_);
        std::swap(data_, temp.data_);
        std::swap(size_, temp.size_);
        std::swap(capacity_, temp.capacity_);
        return *this;
      }

      const size_t common_size = std::min(size_, other.size_);

      for (size_t i = 0; i < common_size; ++i) {
        data_[i] = std::move(other.data_[i]);
      }

      if (size_ > other.size_) {
        destroy_elements(data_ + other.size_, size_ - other.size_);
        size_ = other.size_;
      } else {
        const size_t old_size = size_;

        try {
          for (; size_ < other.size_; ++size_) {
            AllocTraits::construct(allocator_, data_ + size_, std::move(other.data_[size_]));
          }
        } catch (...) {
          destroy_elements(data_ + old_size, size_ - old_size);
          size_ = old_size;
          throw;
        }
      }
    }
  }

  return *this;
}

template <typename T, typename Allocator>
void Vector<T, Allocator>::reserve(size_t newcap) {
  if (newcap <= capacity_) {
    return;
  }

  T* newdata = AllocTraits::allocate(allocator_, newcap);
  size_t index = 0;

  try {
    for (; index < size_; ++index) {
      AllocTraits::construct(allocator_, newdata + index, std::move_if_noexcept(data_[index]));
    }
  } catch (...) {
    destroy_elements(newdata, index);
    AllocTraits::deallocate(allocator_, newdata, newcap);
    throw;
  }

  destroy_elements(data_, size_);

  if (data_ != nullptr) {
    AllocTraits::deallocate(allocator_, data_, capacity_);
  }

  data_ = newdata;
  capacity_ = newcap;
}

template <typename T, typename Allocator>
void Vector<T, Allocator>::push_back(const T& value) {
  if (size_ < capacity_) {
    AllocTraits::construct(allocator_, data_ + size_, value);
    ++size_;
    return;
  }

  reallocate_and_push(value);
}

template <typename T, typename Allocator>
void Vector<T, Allocator>::push_back(T&& value) {
  if (size_ < capacity_) {
    AllocTraits::construct(allocator_, data_ + size_, std::move(value));
    ++size_;
    return;
  }

  reallocate_and_push(std::move(value));
}

template <typename T, typename Allocator>
size_t Vector<T, Allocator>::size() const noexcept {
  return size_;
}

template <typename T, typename Allocator>
size_t Vector<T, Allocator>::capacity() const noexcept {
  return capacity_;
}

template <typename T, typename Allocator>
T* Vector<T, Allocator>::data() noexcept {
  return data_;
}

template <typename T, typename Allocator>
const T* Vector<T, Allocator>::data() const noexcept {
  return data_;
}

template <typename T, typename Allocator>
bool Vector<T, Allocator>::empty() const noexcept {
  return size_ == 0;
}

template <typename T, typename Allocator>
void Vector<T, Allocator>::swap(Vector& other) noexcept(AllocTraits::propagate_on_container_swap::value ||
                                                        AllocTraits::is_always_equal::value) {
  if constexpr (AllocTraits::propagate_on_container_swap::value) {
    using std::swap;
    swap(allocator_, other.allocator_);
  } else {
    assert(allocator_ == other.allocator_);
  }

  std::swap(data_, other.data_);
  std::swap(size_, other.size_);
  std::swap(capacity_, other.capacity_);
}

template <typename T, typename Allocator>
void Vector<T, Allocator>::clear() noexcept {
  destroy_elements(data_, size_);
  size_ = 0;
}

template <typename T, typename Allocator>
void Vector<T, Allocator>::pop_back() noexcept {
  --size_;
  AllocTraits::destroy(allocator_, data_ + size_);
}

template <typename T, typename Allocator>
void Vector<T, Allocator>::resize(size_t newsize) {
  if (newsize == size_) {
    return;
  }

  if (newsize < size_) {
    destroy_elements(data_ + newsize, size_ - newsize);
    size_ = newsize;
    return;
  }

  if (newsize <= capacity_) {
    size_t index = size_;

    try {
      for (; index < newsize; ++index) {
        AllocTraits::construct(allocator_, data_ + index);
      }
    } catch (...) {
      destroy_elements(data_ + size_, index - size_);
      throw;
    }

    size_ = newsize;
    return;
  }

  T* newdata = AllocTraits::allocate(allocator_, newsize);
  size_t new_index = size_;
  size_t old_index = 0;

  try {
    for (; new_index < newsize; ++new_index) {
      AllocTraits::construct(allocator_, newdata + new_index);
    }

    for (; old_index < size_; ++old_index) {
      AllocTraits::construct(allocator_, newdata + old_index, std::move_if_noexcept(data_[old_index]));
    }
  } catch (...) {
    destroy_elements(newdata, old_index);
    destroy_elements(newdata + size_, new_index - size_);
    AllocTraits::deallocate(allocator_, newdata, newsize);
    throw;
  }

  destroy_elements(data_, size_);

  if (data_ != nullptr) {
    AllocTraits::deallocate(allocator_, data_, capacity_);
  }

  data_ = newdata;
  capacity_ = newsize;
  size_ = newsize;
}

template <typename T, typename Allocator>
void Vector<T, Allocator>::shrink_to_fit() {
  if (capacity_ == size_) {
    return;
  }

  if (size_ == 0) {
    AllocTraits::deallocate(allocator_, data_, capacity_);
    data_ = nullptr;
    capacity_ = 0;
    return;
  }

  T* newdata = AllocTraits::allocate(allocator_, size_);
  size_t index = 0;

  try {
    for (; index < size_; ++index) {
      AllocTraits::construct(allocator_, newdata + index, std::move_if_noexcept(data_[index]));
    }
  } catch (...) {
    destroy_elements(newdata, index);
    AllocTraits::deallocate(allocator_, newdata, size_);
    throw;
  }

  destroy_elements(data_, size_);

  if (data_ != nullptr) {
    AllocTraits::deallocate(allocator_, data_, capacity_);
  }

  capacity_ = size_;
  data_ = newdata;
}

template <typename T, typename Allocator>
T& Vector<T, Allocator>::front() noexcept {
  return *data_;
}

template <typename T, typename Allocator>
const T& Vector<T, Allocator>::front() const noexcept {
  return *data_;
}

template <typename T, typename Allocator>
T& Vector<T, Allocator>::back() noexcept {
  return data_[size_ - 1];
}

template <typename T, typename Allocator>
const T& Vector<T, Allocator>::back() const noexcept {
  return data_[size_ - 1];
}

template <typename T, typename Allocator>
T& Vector<T, Allocator>::operator[](size_t index) noexcept {
  return data_[index];
}

template <typename T, typename Allocator>
const T& Vector<T, Allocator>::operator[](size_t index) const noexcept {
  return data_[index];
}

template <typename T, typename Allocator>
T& Vector<T, Allocator>::at(size_t index) {
  if (index >= size_) {
    throw std::out_of_range("Index is out of range");
  }

  return data_[index];
}

template <typename T, typename Allocator>
const T& Vector<T, Allocator>::at(size_t index) const {
  if (index >= size_) {
    throw std::out_of_range("Index is out of range");
  }

  return data_[index];
}

template <typename T, typename Allocator>
template <typename... Args>
T& Vector<T, Allocator>::emplace_back(Args&&... args) {
  if (size_ < capacity_) {
    AllocTraits::construct(allocator_, data_ + size_, std::forward<Args>(args)...);
    ++size_;
    return data_[size_ - 1];
  }

  return reallocate_and_emplace(std::forward<Args>(args)...);
}

template <typename T, typename Allocator>
typename Vector<T, Allocator>::iterator Vector<T, Allocator>::begin() noexcept {
  return data_;
}

template <typename T, typename Allocator>
typename Vector<T, Allocator>::iterator Vector<T, Allocator>::end() noexcept {
  return data_ + size_;
}

template <typename T, typename Allocator>
typename Vector<T, Allocator>::const_iterator Vector<T, Allocator>::begin() const noexcept {
  return data_;
}

template <typename T, typename Allocator>
typename Vector<T, Allocator>::const_iterator Vector<T, Allocator>::end() const noexcept {
  return data_ + size_;
}

template <typename T, typename Allocator>
typename Vector<T, Allocator>::const_iterator Vector<T, Allocator>::cbegin() const noexcept {
  return data_;
}

template <typename T, typename Allocator>
typename Vector<T, Allocator>::const_iterator Vector<T, Allocator>::cend() const noexcept {
  return data_ + size_;
}

template <typename T, typename Allocator>
typename Vector<T, Allocator>::reverse_iterator Vector<T, Allocator>::rbegin() noexcept {
  return reverse_iterator(end());
}

template <typename T, typename Allocator>
typename Vector<T, Allocator>::reverse_iterator Vector<T, Allocator>::rend() noexcept {
  return reverse_iterator(begin());
}

template <typename T, typename Allocator>
typename Vector<T, Allocator>::const_reverse_iterator Vector<T, Allocator>::rbegin() const noexcept {
  return const_reverse_iterator(end());
}

template <typename T, typename Allocator>
typename Vector<T, Allocator>::const_reverse_iterator Vector<T, Allocator>::rend() const noexcept {
  return const_reverse_iterator(begin());
}

template <typename T, typename Allocator>
typename Vector<T, Allocator>::const_reverse_iterator Vector<T, Allocator>::crbegin() const noexcept {
  return const_reverse_iterator(cend());
}

template <typename T, typename Allocator>
typename Vector<T, Allocator>::const_reverse_iterator Vector<T, Allocator>::crend() const noexcept {
  return const_reverse_iterator(cbegin());
}

template <typename T, typename Allocator>
typename Vector<T, Allocator>::iterator Vector<T, Allocator>::insert(const_iterator pos, size_t count, const T& value) {
  size_t pos_ind = pos - cbegin();

  if (count == 0) {
    return data_ + pos_ind;
  }

  if (capacity_ >= size_ + count) {
    const size_t old_size = size_;
    const size_t elements_after = old_size - pos_ind;
    size_t constructed = 0;

    try {
      if (elements_after == 0) {
        for (; constructed < count; ++constructed) {
          AllocTraits::construct(allocator_, data_ + old_size + constructed, value);
        }
      } else if (elements_after >= count) {
        T value_copy = value;

        for (size_t i = old_size - count; i < old_size; ++i) {
          AllocTraits::construct(allocator_, data_ + i + count, std::move_if_noexcept(data_[i]));
          ++constructed;
        }

        for (size_t i = old_size - count; i-- > pos_ind;) {
          data_[i + count] = std::move(data_[i]);
        }

        for (size_t i = pos_ind; i < pos_ind + count; ++i) {
          data_[i] = value_copy;
        }
      } else {
        T value_copy = value;
        const size_t extra = count - elements_after;

        for (size_t i = 0; i < extra; ++i) {
          AllocTraits::construct(allocator_, data_ + old_size + i, value_copy);
          ++constructed;
        }

        for (size_t i = pos_ind; i < old_size; ++i) {
          AllocTraits::construct(allocator_, data_ + i + count, std::move_if_noexcept(data_[i]));
          ++constructed;
        }

        for (size_t i = pos_ind; i < old_size; ++i) {
          data_[i] = value_copy;
        }
      }
    } catch (...) {
      destroy_elements(data_ + old_size, constructed);
      throw;
    }

    size_ = old_size + count;
    return data_ + pos_ind;
  }

  const size_t newcap = std::max(size_ + count, capacity_ == 0 ? size_t{1} : capacity_ * 2);
  T* newdata = AllocTraits::allocate(allocator_, newcap);
  size_t count_index = 0;
  size_t index = 0;

  try {
    for (; count_index < count; ++count_index) {
      AllocTraits::construct(allocator_, newdata + pos_ind + count_index, value);
    }

    for (; index < pos_ind; ++index) {
      AllocTraits::construct(allocator_, newdata + index, std::move_if_noexcept(data_[index]));
    }

    for (; index < size_; ++index) {
      AllocTraits::construct(allocator_, newdata + count + index, std::move_if_noexcept(data_[index]));
    }
  } catch (...) {
    destroy_elements(newdata + pos_ind, count_index);

    if (index < pos_ind) {
      destroy_elements(newdata, index);
    } else {
      destroy_elements(newdata, pos_ind);
      destroy_elements(newdata + pos_ind + count, index - pos_ind);
    }

    AllocTraits::deallocate(allocator_, newdata, newcap);
    throw;
  }

  destroy_elements(data_, size_);

  if (data_ != nullptr) {
    AllocTraits::deallocate(allocator_, data_, capacity_);
  }

  data_ = newdata;
  size_ += count;
  capacity_ = newcap;
  return data_ + pos_ind;
}

template <typename T, typename Allocator>
typename Vector<T, Allocator>::iterator Vector<T, Allocator>::insert(const_iterator pos, const T& value) {
  return insert(pos, 1, value);
}

template <typename T, typename Allocator>
typename Vector<T, Allocator>::iterator Vector<T, Allocator>::insert(const_iterator pos, T&& value) {
  const size_t pos_ind = pos - cbegin();

  if (size_ < capacity_) {
    if (pos_ind == size_) {
      AllocTraits::construct(allocator_, data_ + size_, std::move(value));
      ++size_;
      return data_ + pos_ind;
    }

    T value_move = std::move(value);
    bool tail_constructed = false;

    try {
      AllocTraits::construct(allocator_, data_ + size_, std::move_if_noexcept(data_[size_ - 1]));
      tail_constructed = true;

      for (size_t i = size_ - 1; i-- > pos_ind;) {
        data_[i + 1] = std::move(data_[i]);
      }

      data_[pos_ind] = std::move(value_move);
    } catch (...) {
      if (tail_constructed) {
        AllocTraits::destroy(allocator_, data_ + size_);
      }
      throw;
    }

    ++size_;
    return data_ + pos_ind;
  }

  const size_t newcap = std::max(size_ + 1, capacity_ == 0 ? size_t{1} : capacity_ * 2);
  T* newdata = AllocTraits::allocate(allocator_, newcap);
  size_t index = 0;
  bool inserted = false;

  try {
    AllocTraits::construct(allocator_, newdata + pos_ind, std::move(value));
    inserted = true;

    for (; index < pos_ind; ++index) {
      AllocTraits::construct(allocator_, newdata + index, std::move_if_noexcept(data_[index]));
    }

    for (; index < size_; ++index) {
      AllocTraits::construct(allocator_, newdata + index + 1, std::move_if_noexcept(data_[index]));
    }
  } catch (...) {
    if (inserted) {
      AllocTraits::destroy(allocator_, newdata + pos_ind);
    }

    if (index < pos_ind) {
      destroy_elements(newdata, index);
    } else {
      destroy_elements(newdata, pos_ind);
      destroy_elements(newdata + pos_ind + 1, index - pos_ind);
    }

    AllocTraits::deallocate(allocator_, newdata, newcap);
    throw;
  }

  destroy_elements(data_, size_);

  if (data_ != nullptr) {
    AllocTraits::deallocate(allocator_, data_, capacity_);
  }

  data_ = newdata;
  capacity_ = newcap;
  ++size_;
  return data_ + pos_ind;
}

template <typename T, typename Allocator>
typename Vector<T, Allocator>::iterator Vector<T, Allocator>::erase(const_iterator first, const_iterator last) {
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

template <typename T, typename Allocator>
typename Vector<T, Allocator>::iterator Vector<T, Allocator>::erase(const_iterator pos) {
  return erase(pos, pos + 1);
}

template <typename T, typename Allocator>
bool operator==(const Vector<T, Allocator>& left, const Vector<T, Allocator>& right) {
  if (left.size() != right.size()) {
    return false;
  }

  for (size_t i = 0; i < left.size(); ++i) {
    if (!(left[i] == right[i])) {
      return false;
    }
  }

  return true;
}

template <typename T, typename Allocator>
Allocator Vector<T, Allocator>::get_allocator() const {
  return allocator_;
}

template <typename T, typename Allocator>
void swap(Vector<T, Allocator>& left, Vector<T, Allocator>& right) noexcept(noexcept(left.swap(right))) {
  left.swap(right);
}
