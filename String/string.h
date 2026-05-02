#include <iostream>
#include <cstring>

class String {
private:
  char* arr_;
  size_t sz_;
  size_t cap_;

  String(size_t len) : arr_(new char[len + 1]), sz_(len), cap_(len + 1) {}

  void reallocate(size_t newcap) {
    char* newdata = new char[newcap];
    memcpy(newdata, arr_, sz_ + 1);
    delete[] arr_;
    arr_ = newdata;
    cap_ = newcap;
  }

public:
  String() : String(static_cast<size_t>(0)) {
    arr_[0] = 0;
  }

  ~String() {
    delete[] arr_;
  }

  String(const char* str) : String(strlen(str)) {
    memcpy(arr_, str, sz_ + 1);
  }

  String(size_t len, char symbol) : String(len) {
    memset(arr_, symbol, sz_);
    arr_[len] = 0;
  }

  String(const String& str) : String(str.sz_) {
    memcpy(arr_, str.arr_, sz_ + 1);
  }

  String& operator=(const String& str) {
    if (this == &str) {
      return *this;
    }

    if (cap_ < str.sz_ + 1) {
      delete[] arr_;
      arr_ = new char[str.sz_];
      cap_ = str.cap_;
    }

    sz_ = str.sz_;
    memcpy(arr_, str.arr_, sz_ + 1);
    return *this;
  }

  friend bool operator==(const String&, const String&);

  friend bool operator>(const String&, const String&);

  char& operator[](size_t ind) {
    return arr_[ind];
  }

  const char& operator[](size_t ind) const {
    return arr_[ind];
  }

  size_t length() const {
    return sz_;
  }

  size_t size() const {
    return sz_;
  }

  size_t capacity() const {
    return cap_ - 1;
  }

  void push_back(char symbol) {
    if (cap_ == sz_ + 1) {
      reallocate(2 * cap_);
    }

    arr_[sz_] = symbol;
    arr_[sz_ + 1] = 0;
    sz_++;
  }

  void pop_back() {
    arr_[sz_ - 1] = 0;
    sz_--;
  }

  char& front() {
    return arr_[0];
  }

  const char& front() const {
    return arr_[0];
  }

  char& back() {
    return arr_[sz_ - 1];
  }

  const char& back() const {
    return arr_[sz_ - 1];
  }

  String& operator+=(const String& str) {
    if (sz_ + str.sz_ + 1 > cap_) {
      reallocate(sz_ + str.sz_ + 1);
    }

    memcpy(arr_ + sz_, str.arr_, str.sz_ + 1);
    sz_ += str.sz_;
    return *this;
  }

  String& operator+=(char symbol) {
    push_back(symbol);
    return *this;
  }

  String operator+(const String& str) const {
    String newstr;
    newstr += *this;
    newstr += str;
    return newstr;
  }

  String operator+(char symbol) const {
    String newstr;
    newstr += *this;
    newstr += symbol;
    return newstr;
  }

  std::pair<size_t, size_t> lrfind(const String& substring) const {
    std::pair<size_t, size_t> result = {sz_, sz_};

    for (size_t i = 0; i + substring.sz_ < sz_ + 1; i++) {
      bool check = true;

      for (size_t j = 0; j < substring.sz_; j++) {
        if (substring.arr_[j] != arr_[i + j]) {
          check = false;
          break;
        }
      }

      if (check) {
        result.first = std::min(result.first, i);
        result.second = i;
      }
    }

    return result;
  }

  size_t find(const String& substring) const {
    return lrfind(substring).first;
  }

  size_t rfind(const String& substring) const {
    return lrfind(substring).second;
  }

  String substr(size_t start, size_t count) const {
    String newstr(count);
    memcpy(newstr.arr_, arr_ + start, count);
    newstr[count] = 0;
    return newstr;
  }

  bool empty() const {
    return sz_ == 0;
  }

  void clear() {
    sz_ = 0;
    arr_[0] = 0;
  }

  void shrink_to_fit() {
    reallocate(sz_ + 1);
  }

  char* data() {
    return arr_;
  }

  const char* data() const {
    return arr_;
  }

  friend String operator+(char, const String&);

  friend std::ostream& operator<<(std::ostream&, const String&);
};

String operator+(char symbol, const String& str) {
  String newstr;
  newstr += symbol;
  newstr += str;
  return newstr;
}

std::ostream& operator<<(std::ostream& out, const String& str) {
  out << str.arr_;
  return out;
}

std::istream& operator>>(std::istream& in, String& str) {
  char sym = in.get();
  str.clear();

  while (isspace(sym) && sym != 0 && sym != -1) {
    sym = in.get();
  }

  while (!isspace(sym) && sym != 0 && sym != -1) {
    str += sym;
    sym = in.get();
  }

  return in;
}

bool operator==(const String& str1, const String& str2) {
  return strcmp(str1.arr_, str2.arr_) == 0;
}

bool operator!=(const String& str1, const String& str2) {
  return !(str1 == str2);
}

bool operator>(const String& str1, const String& str2) {
  return strcmp(str1.arr_, str2.arr_) > 0;
}

bool operator<(const String& str1, const String& str2) {
  return str2 > str1;
}

bool operator>=(const String& str1, const String& str2) {
  return !(str1 < str2);
}

bool operator<=(const String& str1, const String& str2) {
  return !(str1 > str2);
}
