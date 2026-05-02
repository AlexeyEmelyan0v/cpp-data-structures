#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <iomanip>

const int double_precision = 16;
const int base = 1000;
const int tenth_power = 3;

enum class Sign {
  negative = -1,
  zero = 0,
  positive = 1
};

int GetIntFromSign(Sign elem) {
  return static_cast<int>(elem);
}

Sign GetSignFromInt(int num) {
  return static_cast<Sign>(num);
}

class BigInteger {
 private:
  std::vector<long long> data_;
  Sign sign_;
 public:
  BigInteger() : data_({0}), sign_(Sign::zero) {};

  void SignIfZero();

  void Shrink();

  void Normalize();

  void ChangeSign();

  BigInteger(int num) : data_({num}), sign_(Sign::positive) {
    if (num < 0) {
      sign_ = Sign::negative;
      data_[0] *= -1;
    }

    Normalize();
  }

  int CompareAbs(const BigInteger&) const;

  BigInteger operator-() const;

  friend bool operator<(const BigInteger&, const BigInteger&);

  BigInteger& operator+=(const BigInteger&);

  BigInteger& operator-=(const BigInteger&);

  BigInteger& operator*=(const BigInteger&);

  BigInteger& operator/=(const BigInteger&);

  BigInteger& operator%=(const BigInteger&);

  BigInteger& operator++();

  BigInteger& operator--();

  BigInteger operator++(int);

  BigInteger operator--(int);

  friend std::ostream& operator<<(std::ostream&, const BigInteger&);

  friend std::istream& operator>>(std::istream&, BigInteger&);

  std::string toString() const;

  explicit operator bool() const;

  friend BigInteger operator ""_bi(unsigned long long num);

  friend BigInteger operator ""_bi(const char* num);

  void Shift(int k);
};

void BigInteger::ChangeSign() {
  if (sign_ == Sign::positive) {
    sign_ = Sign::negative;
    return;
  }
  if (sign_ == Sign::negative) {
    sign_ = Sign::positive;
  }
}

void BigInteger::SignIfZero() {
  if (data_.size() == 1 && data_[0] == 0) {
    sign_ = Sign::zero;
  }
}

void BigInteger::Shrink() {
  while (data_.size() > 1 && data_[data_.size() - 1] == 0) {
    data_.pop_back();
  }

  SignIfZero();
}

void BigInteger::Normalize() {
  for (size_t i = 0; i < data_.size() - 1; i++) {
    if (data_[i] >= base) {
      data_[i + 1] += data_[i] / base;
      data_[i] %= base;
    }

    if (data_[i] < 0) {
      int rest = (data_[i] % base + base) % base;
      data_[i + 1] += (data_[i] - rest) / base;
      data_[i] = rest;
    }
  }

  while (data_[data_.size() - 1] >= base) {
    data_.push_back(data_[data_.size() - 1] / base);
    data_[data_.size() - 2] %= base;
  }

  Shrink();
  SignIfZero();
}

int BigInteger::CompareAbs(const BigInteger& num) const {
  if (data_.size() == num.data_.size()) {
    int ind = data_.size() - 1;

    while (ind >= 0 && data_[ind] == num.data_[ind]) {
      ind--;
    }

    if (ind == -1) {
      return 0;
    }

    return (data_[ind] > num.data_[ind] ? 1 : -1);
  }

  return (data_.size() > num.data_.size() ? 1 : -1);
}

BigInteger operator+(const BigInteger& num1, const BigInteger& num2) {
  BigInteger result = num1;
  result += num2;
  return result;
}

BigInteger operator-(const BigInteger& num1, const BigInteger& num2) {
  BigInteger result = num1;
  result -= num2;
  return result;
}

BigInteger operator*(const BigInteger& num1, const BigInteger& num2) {
  BigInteger result = num1;
  result *= num2;
  return result;
}

BigInteger BigInteger::operator-() const {
  BigInteger result = *this;
  result.ChangeSign();
  return result;
}

std::ostream& operator<<(std::ostream& out, const BigInteger& num) {
  if (num.sign_ == Sign::negative) {
    out << '-';
  }

  out << num.data_[num.data_.size() - 1];

  for (int i = num.data_.size() - 2; i >= 0; i--) {
    out << std::setfill('0') << std::setw(tenth_power) << num.data_[i];
  }

  return out;
}

bool operator<(const BigInteger& num1, const BigInteger& num2) {
  if (num1.sign_ != num2.sign_) {
    return num1.sign_ < num2.sign_;
  }

  if (num1.sign_ == Sign::zero) {
    return false;
  }

  return (num1.CompareAbs(num2) == 1 && num1.sign_ == Sign::negative) ||
      (num1.CompareAbs(num2) == -1 && num1.sign_ == Sign::positive);
}

bool operator>(const BigInteger& num1, const BigInteger& num2) {
  return num2 < num1;
}

bool operator==(const BigInteger& num1, const BigInteger& num2) {
  return !(num2 < num1) && !(num1 < num2);
}

bool operator!=(const BigInteger& num1, const BigInteger& num2) {
  return (num1 < num2) || (num2 < num1);
}

bool operator>=(const BigInteger& num1, const BigInteger& num2) {
  return !(num1 < num2);
}

bool operator<=(const BigInteger& num1, const BigInteger& num2) {
  return !(num2 < num1);
}

BigInteger& BigInteger::operator+=(const BigInteger& num) {
  data_.resize(std::max(data_.size(), num.data_.size()));
  int coef1 = 1;
  int coef2 = 1;
  int cmp = CompareAbs(num);

  if (sign_ != num.sign_) {
    if (cmp == 0) {
      *this = 0;
      return *this;
    }

    if (cmp > 0) {
      coef2 = -1;
    }

    if (cmp < 0) {
      sign_ = num.sign_;
      coef1 = -1;
    }
  }

  for (size_t i = 0; i < data_.size(); i++) {
    data_[i] = coef1 * data_[i];
    if (i < num.data_.size()) {
      data_[i] += coef2 * num.data_[i];
    }
  }

  Normalize();
  return *this;
};

BigInteger& BigInteger::operator*=(const BigInteger& num) {
  BigInteger result;
  result.data_.resize(data_.size() + num.data_.size());
  result.sign_ = GetSignFromInt(GetIntFromSign(sign_) * GetIntFromSign(num.sign_));

  for (size_t i = 0; i < data_.size(); i++) {
    for (size_t j = 0; j < num.data_.size(); j++) {
      result.data_[i + j] += data_[i] * num.data_[j];
    }
  }

  result.Normalize();
  *this = result;
  return *this;
};

BigInteger& BigInteger::operator-=(const BigInteger& num) {
  *this += (-num);
  return *this;
};

BigInteger& BigInteger::operator/=(const BigInteger& num) {
  if (num.data_.size() > data_.size()) {
    *this = 0;
    return *this;
  }

  BigInteger num1 = *this;
  BigInteger num2 = num;
  BigInteger result;
  result.data_.resize(num1.data_.size() - num2.data_.size() + 1);
  num1.sign_ = Sign::positive;
  num2.sign_ = Sign::positive;
  result.sign_ = Sign::positive;
  BigInteger prod = 0;

  for (int i = result.data_.size() - 1; i >= 0; i--) {
    int left = 0;
    int right = base;

    while (right - left > 1) {
      int mid = (right + left) / 2;
      BigInteger num3 = num2 * mid;
      num3.Shift(i);

      if (prod + num3 > num1) {
        right = mid;
      } else {
        left = mid;
      }
    }

    BigInteger num3 = num2 * left;
    num3.Shift(i);
    prod += num3;
    result.data_[i] = left;
  }

  result.sign_ = GetSignFromInt(GetIntFromSign(sign_) * GetIntFromSign(num.sign_));
  result.Normalize();
  *this = result;
  return *this;
}

BigInteger BigInteger::operator++(int) {
  BigInteger result = *this;
  *this += 1;
  return result;
}

BigInteger BigInteger::operator--(int) {
  BigInteger result = *this;
  *this -= 1;
  return result;
}

void BigInteger::Shift(int k) {
  if (k == 0) {
    return;
  }

  int sz = data_.size();
  data_.resize(sz + k);

  for (int i = sz - 1; i >= 0; i--) {
    data_[i + k] = data_[i];
    data_[i] = 0;
  }
}

BigInteger operator/(const BigInteger& num1, const BigInteger& num2) {
  BigInteger result = num1;
  result /= num2;
  return result;
}

BigInteger operator%(const BigInteger& num1, const BigInteger& num2) {
  BigInteger result = num1;
  result %= num2;
  return result;
}

BigInteger& BigInteger::operator%=(const BigInteger& num) {
  *this -= (*this / num) * num;
  return *this;
}

BigInteger& BigInteger::operator--() {
  if (sign_ == Sign::positive) {
    data_[0]--;
  } else if (sign_ == Sign::zero) {
    data_[0]++;
    sign_ = Sign::negative;
  } else {
    data_[0]++;
  }

  if (data_[0] < 0 || data_[0] >= base) {
    Normalize();
  }

  SignIfZero();

  return *this;
}

BigInteger& BigInteger::operator++() {
  if (sign_ == Sign::positive) {
    data_[0]++;
  } else if (sign_ == Sign::zero) {
    data_[0]++;
    sign_ = Sign::positive;
  } else {
    data_[0]--;
  }

  if (data_[0] >= base || data_[0] < 0) {
    Normalize();
  }

  SignIfZero();

  return *this;
}

std::istream& operator>>(std::istream& in, BigInteger& num) {
  std::string str;
  in >> str;
  int start = 0;
  num.sign_ = Sign::positive;

  if (str[0] == '0') {
    num.sign_ = Sign::zero;
    num.data_.resize(1);
    num.data_[0] = 0;
    return in;
  }

  if (str[0] == '-') {
    num.sign_ = Sign::negative;
    start = 1;
  }

  num.data_.resize(0);

  for (int i = str.size() - 3; i + 2 >= start; i -= 3) {
    std::string part;

    if (i >= start) {
      part += str[i];
    }

    if (i + 1 >= start) {
      part += str[i + 1];
    }

    part += str[i + 2];

    num.data_.push_back(std::stoi(part));
  }

  return in;
}

std::string BigInteger::toString() const {
  std::string result;

  if (sign_ == Sign::negative) {
    result += '-';
  }

  result += std::to_string(data_[data_.size() - 1]);

  for (int i = data_.size() - 2; i >= 0; i--) {
    std::string num = std::to_string(data_[i]);
    std::string zeros(tenth_power - num.size(), '0');
    result += zeros;
    result += num;
  }

  return result;
}

BigInteger::operator bool() const {
  return sign_ != Sign::zero;
}

BigInteger operator ""_bi(unsigned long long num) {
  BigInteger result;
  result.data_.resize(0);

  if (num == 0) {
    return 0;
  }

  while (num != 0) {
    result.data_.push_back(num % base);
    num /= base;
  }

  return result;
}

BigInteger operator ""_bi(const char* str) {
  BigInteger num;
  int start = 0;
  num.sign_ = Sign::positive;

  if (str[0] == '0') {
    return 0;
  }

  if (str[0] == '-') {
    num.sign_ = Sign::negative;
    start = 1;
  }

  num.data_.resize(0);

  for (int i = strlen(str) - 3; i + 2 >= start; i -= 3) {
    std::string part;

    if (i >= start) {
      part += str[i];
    }

    if (i + 1 >= start) {
      part += str[i + 1];
    }

    part += str[i + 2];

    num.data_.push_back(std::stoi(part));
  }

  return num;
}

BigInteger gcd(const BigInteger& num1, const BigInteger& num2) {
  if (num1 < 0) {
    return gcd(-num1, num2);
  }

  if (num2 == 0) {
    return num1;
  }

  return gcd(num2, num1 % num2);
}

class Rational {
 private:
  BigInteger num_;
  BigInteger den_;
 public:
  Rational() : num_(0), den_(1) {};

  void Normalize();

  Rational(const BigInteger& num) : num_(num), den_(1) {}

  Rational(const BigInteger& num, const BigInteger& den) : num_(num), den_(den) {
    Normalize();
  }

  Rational(int num) : num_(num), den_(1) {};

  Rational& operator+=(const Rational&);

  Rational& operator-=(const Rational&);

  Rational& operator*=(const Rational&);

  Rational& operator/=(const Rational&);

  Rational operator-() const;

  friend bool operator<(const Rational&, const Rational&);

  std::string toString() const;

  std::string asDecimal(size_t) const;

  explicit operator double() const;
};

void Rational::Normalize() {
  if (den_ < 0) {
    den_.ChangeSign();
    num_.ChangeSign();
  }

  BigInteger gcdnd = gcd(num_, den_);
  num_ /= gcdnd;
  den_ /= gcdnd;
}

Rational operator+(const Rational& num1, const Rational& num2) {
  Rational result = num1;
  result += num2;
  return result;
}

Rational operator-(const Rational& num1, const Rational& num2) {
  Rational result = num1;
  result -= num2;
  return result;
}

Rational operator*(const Rational& num1, const Rational& num2) {
  Rational result = num1;
  result *= num2;
  return result;
}

Rational operator/(const Rational& num1, const Rational& num2) {
  Rational result = num1;
  result /= num2;
  return result;
}

Rational& Rational::operator+=(const Rational& num) {
  num_ = num_ * num.den_ + num.num_ * den_;
  den_ *= num.den_;
  Normalize();
  return *this;
}

Rational& Rational::operator-=(const Rational& num) {
  num_ = num_ * num.den_ - num.num_ * den_;
  den_ *= num.den_;
  Normalize();
  return *this;
}

Rational& Rational::operator*=(const Rational& num) {
  num_ *= num.num_;
  den_ *= num.den_;
  Normalize();
  return *this;
}

Rational& Rational::operator/=(const Rational& num) {
  num_ *= num.den_;
  den_ *= num.num_;
  Normalize();
  return *this;
}

Rational Rational::operator-() const {
  Rational result = *this;
  result.num_.ChangeSign();
  return result;
}

bool operator<(const Rational& num1, const Rational& num2) {
  return num1.num_ * num2.den_ < num2.num_ * num1.den_;
}

bool operator>(const Rational& num1, const Rational& num2) {
  return num2 < num1;
}

bool operator==(const Rational& num1, const Rational& num2) {
  return !(num1 < num2) && !(num2 < num1);
}

bool operator>=(const Rational& num1, const Rational& num2) {
  return !(num1 < num2);
}

bool operator<=(const Rational& num1, const Rational& num2) {
  return !(num2 < num1);
}

bool operator!=(const Rational& num1, const Rational& num2) {
  return (num1 < num2) || (num2 < num1);
}

std::string Rational::toString() const {
  std::string result;
  result += num_.toString();

  if (den_ != 1) {
    result += '/';
    result += den_.toString();
  }

  return result;
}

std::string Rational::asDecimal(size_t precision) const {
  BigInteger num = num_;
  BigInteger den = den_;
  std::string result;

  if (num < 0) {
    result += '-';
    num.ChangeSign();
  }

  if (num < den) {
    num *= 10;
    result += "0";
  } else {
    result += (num / den).toString();
    num %= den;
    num *= 10;
  }

  if (precision > 0) {
    result += '.';
  }

  for (size_t i = 0; i < precision; i++) {
    if (num >= den) {
      result += (num / den).toString();
      num %= den;
    } else {
      result += '0';
    }

    num *= 10;
  }

  return result;
}

Rational::operator double() const {
  return std::stod(asDecimal(double_precision));
}
