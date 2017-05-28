#pragma once

namespace hoc {

template <typename val_t>
class valuable_t {
public:

  // constructor
  valuable_t() = default;

  valuable_t(val_t val_) {
    val = val_;
  }

  // getter
  virtual val_t get() const {
    return val;
  }

  // arithmetic operators
  const val_t operator+(const val_t &that) const {
    return val + that;
  }

  const val_t operator-(const val_t &that) const {
    return val - that;
  }

  const val_t operator*(const val_t &that) const {
    return val * that;
  }

  const val_t operator/(const val_t &that) const {
    return val / that;
  }

  const val_t operator%(const val_t &that) const {
    return val % that;
  }

  // prefix increment
  friend const val_t &operator++(valuable_t &that) {
    static const val_t temp = ++that.val;
    return temp;
  }

  // postfix increment
  friend const val_t &operator++(valuable_t &that, int) {
    static const val_t temp = that.val++;
    return temp;
  }

  // prefix decrement
  friend const val_t &operator--(valuable_t &that) {
    static const val_t temp = --that.val;
    return temp;
  }

  // postfix decrement
  friend const val_t &operator--(valuable_t &that, int) {
    static const val_t temp = that.val--;
    return temp;
  }

  // assignment
  valuable_t &operator=(const valuable_t<val_t> &that) {
    val = that.get();
    return *this;
  }

  valuable_t &operator=(const val_t &that) {
    val = that;
    return *this;
  }

  // relational
  bool operator==(const val_t &that) {
    return val == that;
  }

  friend bool operator==(const val_t &val_, const valuable_t &that) {
    return val_ == that.get();
  }

  bool operator!=(const val_t &that) {
    return val != that;
  }

  friend bool operator!=(const val_t &val_, const valuable_t &that) {
    return val_ != that.get();
  }

  bool operator>(const val_t &that) {
    return val > that;
  }

  friend bool operator>(const val_t &val_, const valuable_t &that) {
    return val_ > that.get();
  }

  bool operator<(const val_t &that) {
    return val < that;
  }

  friend bool operator<(const val_t &val_, const valuable_t &that) {
    return val_ < that.get();
  }

  bool operator>=(const val_t &that) {
    return val >= that;
  }

  friend bool operator>=(const val_t &val_, const valuable_t &that) {
    return val_ >= that.get();
  }

  bool operator<=(const val_t &that) {
    return val <= that;
  }

  friend bool operator<=(const val_t &val_, const valuable_t &that) {
    return val_ <= that.get();
  }

  bool operator!() const {
    return !val;
  }

  val_t operator~() const {
    return ~val;
  }

  val_t operator+() const {
    return +val;
  }

  val_t operator-() const {
    return -val;
  }

  bool operator&&(const val_t &that) const {
    return val && that;
  }

  friend val_t operator~(const valuable_t &that) {
    return ~that.get();
  }

  friend bool operator&&(const val_t &val_, const valuable_t &that) {
    return val_ && that.get();
  }

  bool operator||(const val_t &that) const {
    return val || that;
  }

  friend bool operator||(const val_t &val_, const valuable_t &that) {
    return val_ || that.get();
  }

  val_t &operator&=(const val_t &that) {
    return val &= that;
  }

  val_t &operator^=(const val_t &that) {
    return val ^= that;
  }

  val_t &operator|=(const val_t &that) {
    return val |= that;
  }

  val_t &operator-=(const val_t &that) {
    return val -= that;
  }

  val_t &operator<<=(const val_t &that) {
    return val <<= that;
  }

  val_t &operator*=(const val_t &that) {
    return val *= that;
  }

  val_t &operator/=(const val_t &that) {
    return val /= that;
  }

  val_t &operator%=(const val_t &that) {
    return val %= that;
  }

  val_t &operator>>=(const val_t &that) {
    return val >>= that;
  }

  val_t &operator+=(const val_t &that) {
    return val += that;
  }

  friend val_t &operator&=(const val_t &val_, const valuable_t &that) {
    return val_ &= that.get();
  }

  friend val_t &operator^=(const val_t &val_, const valuable_t &that) {
    return val_ ^= that.get();
  }

  friend val_t &operator|=(const val_t &val_, const valuable_t &that) {
    return val_ |= that.get();
  }

  friend val_t &operator-=(const val_t &val_, const valuable_t &that) {
    return val_ -= that.get();
  }

  friend val_t &operator<<=(const val_t &val_, const valuable_t &that) {
    return val_ <<= that.get();
  }

  friend val_t &operator*=(const val_t &val_, const valuable_t &that) {
    return val_ *= that.get();
  }

  friend val_t &operator/=(const val_t &val_, const valuable_t &that) {
    return val_ /= that.get();
  }

  friend val_t &operator%=(const val_t &val_, const valuable_t &that) {
    return val_ %= that.get();
  }

  friend val_t &operator>>=(const val_t &val_, const valuable_t &that) {
    return val_ >>= that.get();
  }

  friend val_t &operator+=(const val_t &val_, const valuable_t &that) {
    return val_ += that.get();
  }

  // bitwise operators
  friend val_t operator&(const val_t &val_, const valuable_t &that) {
    return val_ & that.get();
  }

  friend val_t operator^(const val_t &val_, const valuable_t &that) {
    return val_ ^ that.get();
  }

  friend val_t operator|(const val_t &val_, const valuable_t &that) {
    return val_ | that.get();
  }

  friend std::ostream &operator<<(std::ostream &os, const valuable_t<val_t> &that) {
    os << that.get();
    return os;
  }

protected:

  val_t val;

};  // comparable_t<val_t>

}   // hoc