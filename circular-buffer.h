#pragma once

#include <cassert>
#include <cstdlib>
#include <iterator>
#include <memory>
#include <utility>

template <typename T>
class circular_buffer {
  T* data_;
  size_t capacity_;
  size_t head_;
  size_t size_;

  template <class I>
  struct buffer_iterator {
    using value_type = T;
    using reference = I&;
    using pointer = I*;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;

  private:
    circular_buffer* buff_;
    size_t offset_;

    friend circular_buffer;

    size_t get_real_index(size_t off) const noexcept {
      return (offset_ + buff_->head_ + off) % buff_->capacity_;
    }

    buffer_iterator(circular_buffer* buff) noexcept : buff_(buff), offset_(0) {}

  public:
    buffer_iterator() noexcept = default;

    buffer_iterator(circular_buffer* buff, size_t offset) noexcept : buff_(buff), offset_(offset) {}

    template <class J>
    buffer_iterator(const buffer_iterator<J>& other) noexcept
        : buff_(const_cast<circular_buffer*>(other.buff_)),
          offset_(other.offset_) {}

    buffer_iterator& operator++() {
      ++offset_;
      return *this;
    }

    buffer_iterator operator++(int) noexcept {
      buffer_iterator temp = *this;
      ++*this;
      return temp;
    }

    buffer_iterator& operator--() noexcept {
      --offset_;
      return *this;
    }

    buffer_iterator operator--(int) noexcept {
      buffer_iterator temp = *this;
      --*this;
      return temp;
    }

    buffer_iterator operator+(ptrdiff_t offset) const noexcept {
      return buffer_iterator(buff_, offset_ + offset);
    }

    friend buffer_iterator operator+(ptrdiff_t offset, buffer_iterator a) noexcept {
      return buffer_iterator(a.buff_, a.offset_ + offset);
    }

    buffer_iterator operator-(ptrdiff_t offset) const noexcept {
      return *this + (-offset);
    }

    buffer_iterator& operator+=(ptrdiff_t offset) noexcept {
      return *this = *this + offset;
    }

    buffer_iterator& operator-=(ptrdiff_t offset) noexcept {
      return *this = *this - offset;
    }

    ptrdiff_t operator-(const buffer_iterator& other) const noexcept {
      return offset_ - other.offset_;
    }

    reference operator[](size_t offset) const noexcept {
      return *(buff_->data() + get_real_index(offset));
    }

    reference operator*() const noexcept {
      return *(buff_->data() + get_real_index(0));
    }

    pointer operator->() const noexcept {
      return buff_->data() + get_real_index(0);
    }

    bool operator!=(const circular_buffer& other) const noexcept {
      return !(*this == other);
    }

    template <class J>
    bool operator==(const buffer_iterator<J>& other) const noexcept {
      return buff_ == other.buff_ && offset_ == other.offset_;
    }

    template <class J>
    bool operator!=(const buffer_iterator<J>& other) const noexcept {
      return !(*this == other);
    }

    bool operator>(const buffer_iterator& other) const noexcept {
      return buff_ == other.buff_ && offset_ > other.offset_;
    }

    bool operator<(const buffer_iterator& other) const noexcept {
      return !(*this == other || *this > other);
    }

    bool operator>=(const buffer_iterator& other) const noexcept {
      return *this > other || *this == other;
    }

    bool operator<=(const buffer_iterator& other) const noexcept {
      return *this < other || *this == other;
    }
  };

public:
  using value_type = T;

  using reference = T&;
  using const_reference = const T&;

  using pointer = T*;
  using const_pointer = const T*;

  using iterator = buffer_iterator<T>;
  using const_iterator = buffer_iterator<const T>;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:
  // O(1), nothrow
  circular_buffer() noexcept : data_(nullptr), capacity_(0), head_(0), size_(0) {}

  // O(n), strong
  circular_buffer(const circular_buffer& other) : circular_buffer(other, other.size_) {}

  // O(n), strong
  circular_buffer& operator=(const circular_buffer& other) {
    if (*this != other) {
      circular_buffer tmp = circular_buffer(other);
      swap(tmp, *this);
    }
    return *this;
  }

  // O(n), nothrow
  ~circular_buffer() {
    clear();
    operator delete(data());
  }

  pointer data() noexcept {
    return data_;
  }

  const_pointer data() const noexcept {
    return data_;
  }

  // O(1), nothrow
  size_t size() const noexcept {
    return size_;
  }

  // O(1), nothrow
  bool empty() const noexcept {
    return size() == 0;
  }

  // O(1), nothrow
  size_t capacity() const noexcept {
    return capacity_;
  }

  // O(1), nothrow
  iterator begin() noexcept {
    return const_cast<circular_buffer*>(this);
  }

  // O(1), nothrow
  const_iterator begin() const noexcept {
    return const_cast<circular_buffer*>(this);
  }

  // O(1), nothrow
  iterator end() noexcept {
    return {const_cast<circular_buffer*>(this), size_};
  }

  // O(1), nothrow
  const_iterator end() const noexcept {
    return {const_cast<circular_buffer*>(this), size_};
  }

  // O(1), nothrow
  reverse_iterator rbegin() noexcept {
    return static_cast<reverse_iterator>(end());
  }

  // O(1), nothrow
  const_reverse_iterator rbegin() const noexcept {
    return static_cast<reverse_iterator>(end());
  }

  // O(1), nothrow
  reverse_iterator rend() noexcept {
    return static_cast<reverse_iterator>(begin());
  }

  // O(1), nothrow
  const_reverse_iterator rend() const noexcept {
    return static_cast<reverse_iterator>(begin());
  }

  // O(1), nothrow
  T& operator[](size_t index) {
    return data()[(head_ + index) % capacity_];
  }

  // O(1), nothrow
  const T& operator[](size_t index) const {
    return data()[(head_ + index) % capacity_];
  }

  // O(1), nothrow
  T& back() {
    return *(end() - 1);
  }

  // O(1), nothrow
  const T& back() const {
    return *(end() - 1);
  }

  // O(1), nothrow
  T& front() {
    return *begin();
  }

  // O(1), nothrow
  const T& front() const {
    return *begin();
  }

  // O(1), strong
  void push_back(const T& val) {
    if (size() == capacity()) {
      ensure_capacity(capacity() == 0 ? 1 : capacity() * 2);
      push_back(val);
    } else {
      new (data() + tail()) T(val);
      ++size_;
    }
  }

  // O(1), strong
  void push_front(const T& val) {
    if (size() == capacity()) {
      ensure_capacity(capacity() == 0 ? 1 : capacity() * 2);
      push_front(val);
    } else {
      size_t tmp = (head_ + capacity_ - 1) % capacity_;
      new (data() + tmp) T(val);
      head_ = tmp;
      ++size_;
    }
  }

  // O(1), nothrow
  void pop_back() {
    std::destroy_n(end() - 1, 1);
    --size_;
    // --size_;
    // data()[tail()].~T();
  }

  // O(1), nothrow
  void pop_front() {
    --size_;
    data()[head_].~T();
    head_ = (head_ + 1) % capacity_;
  }

  // O(n), strong
  void reserve(size_t desired_capacity) {
    ensure_capacity(desired_capacity);
  }

  // O(n), basic
  iterator insert(const_iterator pos, const T& val) {
    ptrdiff_t delta = pos - begin();
    if (delta < size() / 2) {
      push_front(val);
      iterator spot = begin() + delta;
      for (auto i = begin(); i < spot; i++) {
        std::iter_swap(i, i + 1);
      }
      return spot;
    } else {
      push_back(val);
      iterator spot = begin() + delta;
      for (auto i = end() - 1; i > spot; i--) {
        std::iter_swap(i, i - 1);
      }
      return spot;
    }
  }

  // O(n), basic
  iterator erase(const_iterator pos) {
    return erase(pos, pos + 1);
  }

  // O(n), basic
  iterator erase(const_iterator first, const_iterator last) {
    ptrdiff_t delta = last - first;
    if (end() - last < size_ / 2) {
      for (iterator i = first; i + delta < end(); i++) {
        std::iter_swap(i, i + delta);
      }
      for (auto i = 0; i < delta; i++) {
        pop_back();
      }
      return first;
    } else if (delta != 0) {
      ptrdiff_t diff = first - begin();
      for (iterator i = last - 1; i >= begin() + delta; i--) {
        std::iter_swap(i, i - delta);
      }
      for (auto i = 0; i < delta; i++) {
        pop_front();
      }
      return begin() + diff;
    }
    return first;
  }

  // O(n), nothrow
  void clear() noexcept {
    while (!empty()) {
      pop_back();
    }
  }

  // O(1), nothrow
  friend void swap(circular_buffer& a, circular_buffer& b) noexcept {
    std::swap(a.data_, b.data_);
    std::swap(a.capacity_, b.capacity_);
    std::swap(a.head_, b.head_);
    std::swap(a.size_, b.size_);
  }

  bool operator==(const circular_buffer& other) const {
    return data() == other.data() && head_ == other.head_ && size() == other.size();
  }

  bool operator!=(const circular_buffer& other) const {
    return !(*this == other);
  }

private:
  size_t tail() const {
    return (head_ + size()) % capacity_;
  }

  void ensure_capacity(const size_t new_capacity) {
    if (capacity_ < new_capacity) {
      circular_buffer tmp = circular_buffer(*this, new_capacity);
      swap(tmp, *this);
    }
  }

  circular_buffer(const circular_buffer& other, size_t new_capacity)
      : data_(new_capacity == 0 ? nullptr : static_cast<T*>(operator new(sizeof(T) * new_capacity))),
        capacity_(new_capacity),
        head_(0),
        size_(other.size_) {
    try {

      std::uninitialized_copy(other.begin(), other.end(), data_);
    } catch (...) {
      operator delete(data_);
      throw;
    }
  }
};
