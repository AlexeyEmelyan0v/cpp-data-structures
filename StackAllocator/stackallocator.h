#include <iostream>

template <size_t N>
class StackStorage {
 private:
  char* memory;
  size_t top = 0;
 public:
  StackStorage() {
    memory = new char[N];
  }
  StackStorage(const StackStorage&) = delete;

  char* allocate(size_t, size_t);

  ~StackStorage() {
    delete[] memory;
  }
};

template <size_t N>
char* StackStorage<N>::allocate(size_t sz, size_t alignment) {
  top += (alignment - top % alignment) % alignment;
  top += sz;
  return &memory[top - sz];
}

template <typename T, size_t N>
class StackAllocator {
 public:
  StackStorage<N>* stack;

  StackAllocator() = default;
  StackAllocator(StackStorage<N>& stack) : stack(&stack) {}

  template <typename U>
  StackAllocator(const StackAllocator<U, N>& allocator) : stack(allocator.stack) {}

  template <typename U>
  StackAllocator& operator=(const StackAllocator<U, N>&);

  template <typename U>
  struct rebind {
    using other = StackAllocator<U, N>;
  };

  bool operator==(const StackAllocator<T, N>& allocator) const = default;
  bool operator!=(const StackAllocator<T, N>& allocator) const = default;

  T* allocate(size_t);
  void deallocate(T*, size_t) {};

  using value_type = T;
};

template <typename T, size_t N>
template <typename U>
StackAllocator<T, N>& StackAllocator<T, N>::operator=(const StackAllocator<U, N>& allocator) {
  stack = allocator.stack;
  return *this;
}

template <typename T, size_t N>
T* StackAllocator<T, N>::allocate(size_t sz) {
  return reinterpret_cast<T*>(stack->allocate(sz * sizeof(T), sizeof(T)));
}

template <class T, class Allocator = std::allocator<T>>
class List : Allocator {
 private:
  struct BaseNode {
    BaseNode* prev;
    BaseNode* next;

    BaseNode() : prev(this), next(this) {}
    BaseNode(BaseNode* prev, BaseNode* next) : prev(prev), next(next) {}
  };

  struct Node : BaseNode {
    T value;

    Node(BaseNode* prev, BaseNode* next) : BaseNode(prev, next) {}
    Node(const T& value, BaseNode* prev, BaseNode* next) : BaseNode(prev, next), value(value) {}
  };

  using NodeAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
  using NodeTraits = std::allocator_traits<NodeAlloc>;
  NodeAlloc NodeAllocator;
  BaseNode fakeNode;
  size_t sz;

  template <bool is_const>
  struct my_iterator {
    BaseNode* node;

    using value_type = std::conditional_t<is_const, const T, T>;
    using pointer = std::conditional_t<is_const, const T*, T*>;
    using reference = std::conditional_t<is_const, const T&, T&>;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = int;

    my_iterator() = default;
    my_iterator(const my_iterator<is_const>& it) : node(it.node) {}

    explicit my_iterator(BaseNode* node) : node(node) {}
    explicit my_iterator(const BaseNode* node) : node(const_cast<BaseNode*>(node)) {}

    my_iterator<is_const>& operator++() {
      node = node->next;
      return *this;
    }

    my_iterator<is_const> operator++(int) {
      my_iterator<is_const> copy = *this;
      ++(*this);
      return copy;
    }

    my_iterator<is_const>& operator--() {
      node = node->prev;
      return *this;
    }

    my_iterator<is_const> operator--(int) {
      my_iterator<is_const> copy = *this;
      --(*this);
      return copy;
    }

    my_iterator<is_const>& operator=(const my_iterator<is_const>& it) = default;

    value_type& operator*() const {
      return reinterpret_cast<Node*>(node)->value;
    }

    value_type* operator->() const {
      return &(reinterpret_cast<Node*>(node)->value);
    }

    bool operator==(const my_iterator<is_const>& it) const = default;
    bool operator!=(const my_iterator<is_const>& it) const = default;

    operator my_iterator<true>() const {
      return my_iterator<true>(node);
    }

    ~my_iterator() = default;
  };

 public:
  using iterator = my_iterator<false>;
  using const_iterator = my_iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  List(const Allocator& allocator = Allocator()) : NodeAllocator(allocator), fakeNode(), sz(0) {}
  List(size_t, const T&, const Allocator&);
  List(size_t, const Allocator& = Allocator());
  ~List();

  Allocator get_allocator() const;

  List(const List&);
  List& operator=(const List&);

  size_t size() const;

  void push_back(const T&);
  void push_front(const T&);
  void pop_back();
  void pop_front();

  void insert(const_iterator, const T&);
  void erase(const_iterator);

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

  const_reverse_iterator rbegin() const;
  const_reverse_iterator rend() const;
};

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t sz, const T& value, const Allocator& allocator) : List(allocator) {
  for (size_t i = 0; i < sz; ++i) {
    push_back(value);
  }
}

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t n, const Allocator& allocator) : List(allocator) {
  BaseNode* old = &fakeNode;
  BaseNode* node;

  for (size_t i = 0; i < n; ++i) {
    node = NodeTraits::allocate(NodeAllocator, 1);

    try {
      NodeTraits::construct(NodeAllocator, reinterpret_cast<Node*>(node), node, node);
    } catch (...) {
      NodeTraits::deallocate(NodeAllocator, reinterpret_cast<Node*>(node), 1);
      throw;
    }

    old->next = node;
    node->prev = old;
    old = node;
    ++sz;
  }

  old->next = &fakeNode;
  (&fakeNode)->prev = old;
}

template <typename T, typename Allocator>
List<T, Allocator>::List(const List& list)
    : List(std::allocator_traits<Allocator>::select_on_container_copy_construction(list.get_allocator())) {
  for (const_iterator it = list.begin(); it != list.end(); ++it) {
    push_back(*it);
  }
}

template <typename T, typename Allocator>
List<T, Allocator>::~List() {
  while (sz > 0) {
    pop_front();
  }
}

template <typename T, typename Allocator>
Allocator List<T, Allocator>::get_allocator() const {
  return NodeAllocator;
}

template <typename T, typename Allocator>
List<T, Allocator>& List<T, Allocator>::operator=(const List<T, Allocator>& list) {
  List<T, Allocator> copy = list;

  if constexpr (NodeTraits::propagate_on_container_copy_assignment::value) {
    NodeAllocator = list.get_allocator();
  }

  std::swap(copy.sz, sz);
  std::swap(copy.fakeNode, fakeNode);
  std::swap((&copy.fakeNode)->next->prev, (&fakeNode)->next->prev);
  std::swap((&copy.fakeNode)->prev->next, (&fakeNode)->prev->next);
  return *this;
}

template <typename T, typename Allocator>
size_t List<T, Allocator>::size() const {
  return sz;
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_back(const T& value) {
  insert(end(), value);
}

template <typename T, typename Allocator>
void List<T, Allocator>::push_front(const T& value) {
  insert(begin(), value);
}

template <typename T, typename Allocator>
void List<T, Allocator>::pop_back() {
  erase(std::prev(end()));
}

template <typename T, typename Allocator>
void List<T, Allocator>::pop_front() {
  erase(begin());
}

template <typename T, typename Allocator>
void List<T, Allocator>::insert(const_iterator it, const T& value) {
  BaseNode* next = it.node;
  BaseNode* prev = next->prev;
  BaseNode* node;
  node = NodeTraits::allocate(NodeAllocator, 1);

  try {
    NodeTraits::construct(NodeAllocator, reinterpret_cast<Node*>(node), value, next, prev);
  } catch (...) {
    NodeTraits::deallocate(NodeAllocator, reinterpret_cast<Node*>(node), 1);
    throw;
  }

  ++sz;
  node->prev = prev;
  prev->next = node;
  node->next = next;
  next->prev = node;
}

template <typename T, typename Allocator>
void List<T, Allocator>::erase(const_iterator it) {
  --sz;
  BaseNode* node = it.node;
  node->next->prev = node->prev;
  node->prev->next = node->next;
  NodeTraits::destroy(NodeAllocator, static_cast<Node*>(node));
  NodeTraits::deallocate(NodeAllocator, static_cast<Node*>(node), 1);
}

template <typename T, typename Allocator>
List<T, Allocator>::iterator List<T, Allocator>::begin() {
  return iterator((&fakeNode)->next);
}

template <typename T, typename Allocator>
List<T, Allocator>::iterator List<T, Allocator>::end() {
  return iterator(&fakeNode);
}

template <typename T, typename Allocator>
List<T, Allocator>::const_iterator List<T, Allocator>::cbegin() const {
  return const_iterator((&fakeNode)->next);
}

template <typename T, typename Allocator>
List<T, Allocator>::const_iterator List<T, Allocator>::cend() const {
  return const_iterator(&fakeNode);
}

template <typename T, typename Allocator>
List<T, Allocator>::const_iterator List<T, Allocator>::begin() const {
  return cbegin();
}

template <typename T, typename Allocator>
List<T, Allocator>::const_iterator List<T, Allocator>::end() const {
  return cend();
}

template <typename T, typename Allocator>
List<T, Allocator>::reverse_iterator List<T, Allocator>::rbegin() {
  return std::make_reverse_iterator(end());
}

template <typename T, typename Allocator>
List<T, Allocator>::reverse_iterator List<T, Allocator>::rend() {
  return std::make_reverse_iterator(begin());
}

template <typename T, typename Allocator>
List<T, Allocator>::const_reverse_iterator List<T, Allocator>::crbegin() const {
  return std::make_reverse_iterator(cend());
}

template <typename T, typename Allocator>
List<T, Allocator>::const_reverse_iterator List<T, Allocator>::crend() const {
  return std::make_reverse_iterator(cbegin());
}

template <typename T, typename Allocator>
List<T, Allocator>::const_reverse_iterator List<T, Allocator>::rbegin() const {
  return crbegin();
}

template <typename T, typename Allocator>
List<T, Allocator>::const_reverse_iterator List<T, Allocator>::rend() const {
  return crend();
}
