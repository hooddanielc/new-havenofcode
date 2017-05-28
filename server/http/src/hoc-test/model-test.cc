#include <hoc-test/test-helper.h>

#include <vector>
#include <functional>
#include <memory>
#include <ostream>
#include <map>
#include <unordered_map>
#include <mutex>
#include <tuple>
#include <mpark/variant.hpp>

#include <hoc/db/connection.h>

using namespace hoc;

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

template <typename obj_t, typename val_t>
class factory_t final {
public:

  std::shared_ptr<obj_t> require(const val_t &val) {
    std::lock_guard<std::mutex> lock { mutex };
    auto pair = weak_ptrs.emplace(val, std::weak_ptr<obj_t> {});
    auto iter = pair.first;
    auto weak_ptr = iter->second;
    auto result = weak_ptr.lock();
    if (!result) {
      try {
        result = std::make_shared<obj_t>();
        weak_ptr = result;
      } catch (...) {
        weak_ptrs.erase(iter);
        throw;
      }
    }
    return result;
  }

  std::shared_ptr<obj_t> require(const obj_t &obj) {
    auto shared_ptr = std::make_shared<obj_t>(obj);
    std::lock_guard<std::mutex> lock { mutex };
    weak_ptrs[obj.get_primary_key().get()] = shared_ptr;
    return shared_ptr;
  }

  static factory_t *get() {
    static factory_t factory;
    return &factory;
  }

private:

  factory_t() = default;

  ~factory_t() = default;

  std::mutex mutex;

  std::unordered_map<val_t, std::weak_ptr<obj_t>> weak_ptrs;

};  // factory_t<obj_t, val_t>

class any_col_t {
public:

  const std::string name;
  any_col_t(const any_col_t &) = delete;
  any_col_t &operator=(const any_col_t &) = delete;
  virtual ~any_col_t() = default;

protected:

  any_col_t(const std::string &name_) : name(name_) {}

};  // any_col_t

template <typename obj_t>
class any_col_of_t: public any_col_t {
public:

  any_col_of_t(const std::string &name_) : any_col_t(name_) {}

  virtual void write(const obj_t *obj, std::ostream &strm) const = 0;

  virtual bool is_primary_key() const {
    return false;
  }

  virtual bool is_foreign_key() const {
    return false;
  }

protected:

  any_col_of_t() = default;

};  // any_col_of_t<obj_t>

template <typename obj_t, typename val_t>
class col_t final: public any_col_of_t<obj_t> {
public:

  using p2m_t = val_t (obj_t::*);

  const p2m_t p2m;

  explicit col_t(p2m_t p2m_)
      : p2m(p2m_) {}

  explicit col_t(const std::string &name_, p2m_t p2m_)
      : any_col_of_t<obj_t>(name_), p2m(p2m_) {}

  virtual void write(const obj_t *obj, std::ostream &strm) const override {
    strm << (obj->*p2m);
  }

  virtual bool is_primary_key() const override {
    return false;
  }

  virtual bool is_foreign_key() const override {
    return false;
  }

};  // col_t<obj_t, val_t>

template <typename val_t>
class primary_key_t: public valuable_t<val_t> {
public:
  using valuable_t<val_t>::operator=;

  const val_t *operator->() const {
    return &this->val;
  }

}; // primary_key_t<obj_t>

template <typename obj_t, typename val_t>
class foreign_key_t: public valuable_t<val_t> {
public:
  using valuable_t<val_t>::operator=;

  foreign_key_t &operator=(const obj_t &other) {
    this->val = other.get_primary_key().get();
    obj = factory_t<obj_t, val_t>::get()->require(other);
    return *this;
  }

  std::shared_ptr<obj_t> operator->() const {
    if (!obj) {
      obj = factory_t<obj_t, val_t>::get()->require(this->val);
    }
    return obj;
  }

  virtual foreign_key_t &operator=(const primary_key_t<val_t> &other) {
    this->val = other.get();
    return *this;
  }

private:

  mutable std::shared_ptr<obj_t> obj;

};  // foreign_key_t<obj_t, val_t>

template <typename obj_t, typename val_t>
class primary_col_t final: public any_col_of_t<obj_t> {
public:

  using p2m_t = primary_key_t<val_t> (obj_t::*);

  const p2m_t p2m;

  explicit primary_col_t(p2m_t p2m_)
      : p2m(p2m_) {}

  explicit primary_col_t(const std::string &name_, p2m_t p2m_)
      : any_col_of_t<obj_t>(name_), p2m(p2m_) {}

  virtual void write(const obj_t *obj, std::ostream &strm) const override {
    strm << (obj->*p2m);
  }

  virtual bool is_primary_key() const override {
    return true;
  }

  virtual bool is_foreign_key() const override {
    return false;
  }

};  // primary_col_t<obj_t, val_t>

template <typename obj_t, typename val_t, typename target_obj_t>
class foreign_col_t final: public any_col_of_t<obj_t> {
public:

  using p2m_t = foreign_key_t<target_obj_t, val_t> (obj_t::*);
  using p2m_target_t = primary_key_t<val_t> (target_obj_t::*);

  explicit foreign_col_t(p2m_t p2m_, p2m_target_t p2m_target_)
      : p2m(p2m_), p2m_target(p2m_target_) {}

  explicit foreign_col_t(const std::string &name_, p2m_t p2m_, p2m_target_t p2m_target_)
      : any_col_of_t<obj_t>(name_), p2m(p2m_), p2m_target(p2m_target_) {}

  virtual void write(const obj_t *obj, std::ostream &strm) const override {
    strm << (obj->*p2m);
  }

  virtual bool is_primary_key() const override {
    return false;
  }

  virtual bool is_foreign_key() const override {
    return true;
  }

  const p2m_t p2m;

  const p2m_target_t p2m_target;

};  // foreign_col_t<obj_t, val_t, target_obj_t, target_val_t>

template <typename obj_t, typename val_t>
std::shared_ptr<col_t<obj_t, val_t>> make_col(
  const std::string &name,
  val_t (obj_t::*p2m)
) {
  return std::make_shared<col_t<obj_t, val_t>>(name, p2m);
}

template <typename obj_t, typename val_t>
std::shared_ptr<primary_col_t<obj_t, val_t>> make_col(
  const std::string &name,
  primary_key_t<val_t> (obj_t::*p2m)
) {
  return std::make_shared<primary_col_t<obj_t, val_t>>(name, p2m);
}

template <typename obj_t, typename val_t, typename target_obj_t>
std::shared_ptr<foreign_col_t<obj_t, val_t, target_obj_t>> make_col(
  const std::string &name,
  foreign_key_t<target_obj_t, val_t> (obj_t::*p2m),
  primary_key_t<val_t> (target_obj_t::*p2m_target)
) {
  return std::make_shared<foreign_col_t<obj_t, val_t, target_obj_t>>(name, p2m, p2m_target);
}

// require primary key for foreign key columns
template <typename obj_t, typename val_t, typename target_obj_t>
std::shared_ptr<foreign_col_t<obj_t, val_t, target_obj_t>> make_col(
  const std::string &name,
  foreign_key_t<target_obj_t, val_t> (obj_t::*p2m)
) = delete;

template <typename obj_t>
class table_t final {
public:

  const std::string name;

  constexpr table_t(const std::string &&name_, std::vector<std::shared_ptr<any_col_of_t<obj_t>>> &&cols_)
      : name(std::move(name_)), cols(std::move(cols_)) {
    assert(!singleton);
    singleton = this;

    for (const auto &col: cols) {
      if (col->is_primary_key()) {
        primary_col_name = col->name;
        break;
      }
    }

    if (primary_col_name.empty()) {
      throw std::runtime_error("table must have at least one primary_key_t");
    }
  }

  ~table_t() {
    singleton = nullptr;
  }

  std::string get_primary_col_name() const noexcept {
    return primary_col_name;
  }

  table_t *get_table() const noexcept {
    assert(singleton);
    return singleton;
  }

  void write(const obj_t *obj, std::ostream &strm) const {
    strm << "table: " << name << std::endl;
    bool first = true;
    for (const auto &col: cols) {
      (first ? strm << "" : strm << "," << std::endl) << " - " << col->name << ": ";
      col->write(obj, strm);
      first = false;
    }
    strm << std::endl;
  }

  /*
   * get_col_name(&model::primary_key_member)
   *
   * Look up the name of a col_t using a member
   * to pointer reference.
   */
  template <typename val_t>
  std::string get_col_name(val_t (obj_t::*p2m)) const {
    for (const auto &col: cols) {
      if (const col_t<obj_t, val_t> *cast = dynamic_cast<const col_t<obj_t, val_t>*>(&*col)) {
        if (cast->p2m == p2m) {
          return cast->name;
        }
      }
    }
    throw std::runtime_error("column does not exist, did you forget to make_col?");
  }

  /*
   * get_col_name(&model::primary_key_member)
   *
   * Look up the name of a primary_col_t using a member
   * to pointer reference.
   */
  template <typename val_t>
  std::string get_col_name(primary_key_t<val_t> (obj_t::*p2m)) const {
    using pri_t = const primary_col_t<obj_t, val_t>*;
    for (const auto &col: cols) {
      if (pri_t cast = dynamic_cast<pri_t>(&*col)) {
        if (cast->p2m == p2m) {
          return cast->name;
        }
      }
    }
    throw std::runtime_error("column does not exist, did you forget to make_col?");
  }

  /*
   * get_col_name(&model::foreign_key_member)
   *
   * Look up the name of a foreign_col_t using a member
   * to pointer reference.
   */
  template <typename target_val_t, typename target_obj_t>
  std::string get_col_name(foreign_key_t<target_obj_t, target_val_t> (obj_t::*p2m)) const {
    using for_t = const foreign_col_t<obj_t, target_val_t, target_obj_t>*;
    for (const auto &col: cols) {
      if (for_t advanced_col = dynamic_cast<for_t>(&*col)) {
        if (advanced_col->p2m == p2m) {
          return advanced_col->name;
        }
      }
    }
    throw std::runtime_error("column does not exist, did you forget to make_col?");
  }

  std::vector<std::shared_ptr<any_col_of_t<obj_t>>> get_cols() const {
    return cols;
  }

private:

  table_t *singleton;
  std::string primary_col_name;
  std::vector<std::shared_ptr<any_col_of_t<obj_t>>> cols;

};  // table_t<obj_t>

template <typename obj_t, typename val_t>
class pqxx_adapter_t {
public:
  const primary_key_t<val_t> (obj_t::*p2m);

  pqxx_adapter_t(primary_key_t<val_t> (obj_t::*p2m_)) :
    p2m(p2m_),
    is_distinct(false),
    limit_rows(0),
    offset_rows(0) {}

  /*
   * distinct()
   * ============================================= */
  pqxx_adapter_t &distinct() {
    is_distinct = true;
    return *this;
  }

  /*
   * limit()
   * ============================================= */
  pqxx_adapter_t &limit(int limit_) {
    limit_rows = limit_;
    return *this;
  }

  /*
   * offset()
   * ============================================= */
  pqxx_adapter_t &offset(int offset_) {
    offset_rows = offset_;
    return *this;
  }

  /*
   * find(value)
   * ============================================= */
  pqxx_adapter_t &find(const val_t &val) {
    find_toks.push_back(str(val));
    return *this;
  }

  /*
   * find_by(key, value)
   * ============================================= */
  pqxx_adapter_t &find_by(const std::string &key, const std::string &val) {
    find_by_toks[key] = str(val);
    return *this;
  }

  // find_by on current obj
  template <typename col_val_t>
  pqxx_adapter_t &find_by(
    col_val_t (obj_t::*p2m),
    const decltype(col_val_t()) &val
  ) {
    auto name = obj_t::table.get_col_name(p2m);
    std::stringstream ss_name;
    ss_name << obj_t::table.name << "." << name;
    std::stringstream ss_val;
    ss_val << val;
    find_by_toks[ss_name.str()] = ss_val.str();
    return *this;
  }

  // find_by on current primary obj
  pqxx_adapter_t &find_by(
    primary_key_t<val_t> (obj_t::*p2m),
    const val_t &val
  ) {
    auto name = obj_t::table.get_col_name(p2m);
    std::stringstream ss_name;
    ss_name << obj_t::table.name << "." << name;
    std::stringstream ss_val;
    ss_val << val;
    find_by_toks[ss_name.str()] = ss_val.str();
    return *this;
  }

  // find_by on current foreign obj
  template <typename target_obj_t, typename col_val_t>
  pqxx_adapter_t &find_by(
    foreign_key_t<target_obj_t, col_val_t> (obj_t::*p2m),
    const decltype(col_val_t()) &val
  ) {
    auto name = obj_t::table.get_col_name(p2m);
    std::stringstream ss_name;
    ss_name << obj_t::table.name << "." << name;
    std::stringstream ss_val;
    ss_val << val;
    find_by_toks[ss_name.str()] = ss_val.str();
    return *this;
  }

  // find_by on other obj
  template <typename other_obj_t, typename other_val_t>
  pqxx_adapter_t &find_by(
    other_val_t (other_obj_t::*p2m),
    const decltype(other_val_t()) &val
  ) {
    auto name = other_obj_t::table.get_col_name(p2m);
    std::stringstream ss_name;
    ss_name << other_obj_t::table.name << "." << name;
    std::stringstream ss_val;
    ss_val << val;
    find_by_toks[ss_name.str()] = ss_val.str();
    return *this;
  }

  // find_by on other primary obj
  template <typename other_obj_t, typename other_val_t>
  pqxx_adapter_t &find_by(
    primary_key_t<other_val_t> (other_obj_t::*p2m),
    const decltype(other_val_t()) &val
  ) {
    auto name = other_obj_t::table.get_col_name(p2m);
    std::stringstream ss_name;
    ss_name << other_obj_t::table.name << "." << name;
    std::stringstream ss_val;
    ss_val << val;
    find_by_toks[ss_name.str()] = ss_val.str();
    return *this;
  }

  // find_by on other foreign obj
  template <typename other_obj_t, typename target_t, typename col_val_t>
  pqxx_adapter_t &find_by(
    foreign_key_t<target_t, col_val_t> (other_obj_t::*p2m),
    const decltype(col_val_t()) &val
  ) {
    auto name = other_obj_t::table.get_col_name(p2m);
    std::stringstream ss_name;
    ss_name << other_obj_t::table.name << "." << name;
    std::stringstream ss_val;
    ss_val << val;
    find_by_toks[ss_name.str()] = ss_val.str();
    return *this;
  }

  /*
   * where(condition)
   * ============================================= */
  pqxx_adapter_t &where(const std::string &condition) {
    where_toks.push_back(condition);
    return *this;
  }

  /*
   * order(key, direction = asc)
   * ============================================= */
  pqxx_adapter_t &order(const std::string &key, const std::string &direction = "") {
    order_toks[key] = direction;
    return *this;
  }

  template <typename other_obj_t, typename col_val_t>
  pqxx_adapter_t &order(
    col_val_t (other_obj_t::*p2m),
    const std::string &direction = ""
  ) {
    auto name = other_obj_t::table.get_col_name(p2m);
    std::stringstream ss_name;
    ss_name << other_obj_t::table.name << "." << name;
    order_toks[ss_name.str()] = direction;
    return *this;
  }

  template <typename other_obj_t, typename col_val_t>
  pqxx_adapter_t &order(
    primary_key_t<val_t> (other_obj_t::*p2m),
    const std::string &direction = ""
  ) {
    auto name = other_obj_t::table.get_col_name(p2m);
    std::stringstream ss_name;
    ss_name << other_obj_t::table.name << "." << name;
    order_toks[ss_name.str()] = direction;
    return *this;
  }

  template <typename other_obj_t, typename target_t, typename col_val_t>
  pqxx_adapter_t &order(
    foreign_key_t<target_t, col_val_t> (other_obj_t::*p2m),
    const std::string &direction = ""
  ) {
    auto name = other_obj_t::table.get_col_name(p2m);
    std::stringstream ss_name;
    ss_name << other_obj_t::table.name << "." << name;
    order_toks[ss_name.str()] = direction;
    return *this;
  }

  /*
   * joins
   * ============================== */
  template <typename arg_t, typename... more_t>
  pqxx_adapter_t &joins(arg_t arg, more_t... more) {
    add_join(arg);
    return joins(more...);
  }

  pqxx_adapter_t &joins() {
    return *this;
  }

  /*
   * print debug output
   * ======================= */
  template <typename list_type_t>
  void print_list(const std::string name, std::vector<list_type_t> &list, std::ostream &strm) {
    strm << "  " << name << std::endl;
    for (const auto &item : list) {
      strm << "    " << item << std::endl;
    }
    strm << std::endl;
  }

  void print_map(const std::string &name, std::unordered_map<std::string, std::string> &list, std::ostream &strm) {
    strm << "  " << name << std::endl;
    for (const auto &item : list) {
      strm << "    " << item.first << " : " << item.second << std::endl;
    }
    strm << std::endl;
  }

  void print_joins(std::ostream &strm) {
    strm << "  " << std::endl;
    for (const auto &item : join_toks) {
      strm << "    " << std::get<0>(item) << std::endl;
      strm << "      " << std::get<1>(item) << std::endl;
      strm << "      " << std::get<2>(item) << std::endl;
      strm << "      " << std::get<3>(item) << std::endl;
    }
    strm << std::endl;
  }

  void write(std::ostream &strm) {
    strm << "pqxx_adapter_t<" << obj_t::table.name << ">" << std::endl;
    print_list("find_toks", find_toks, strm);
    print_list("where_toks", where_toks, strm);
    print_map("find_by_toks", find_by_toks, strm);
    print_map("order_toks", order_toks, strm);
    print_joins(strm);
  }

  // todo create_with
  // todo group
  // todo having
  // todo includes
  // todo references
  // todo like

  /*
   * to_sql();
   * ======================= */
  std::string to_sql(
    std::shared_ptr<pqxx::connection> c = hoc::db::anonymous_connection()
  ) {
    std::stringstream ss;
    ss << "select ";

    if (is_distinct) {
      ss << "distinct ";
    }

    ss << obj_t::table.name << ".* from " << obj_t::table.name << " ";

    // eval joins
    for (const auto &join : join_toks) {
      ss << std::get<0>(join) << " " << std::get<1>(join) << " on "
         << std::get<2>(join) << " " << std::get<3>(join) << " ";
    }

    bool needs_where = find_toks.size() ||
      find_by_toks.size() ||
      where_toks.size() ||
      join_toks.size();

    if (needs_where) {
      ss << "where";

      if (find_toks.size()) {
        ss << " (";
        for (auto it = find_toks.begin(); it != find_toks.end(); ++it) {
          ss << obj_t::table.name << "." << obj_t::table.get_primary_col_name() << " = ";
          ss << c->quote(*it);
          if (std::next(it) != find_toks.end()) {
            ss << " or ";
          }
        }
        ss << ")";
      }

      if (where_toks.size()) {
        if (find_toks.size()) {
          ss << " or (";
        } else {
          ss << " (";
        }
        for (auto it = where_toks.begin(); it != where_toks.end(); ++it) {
          ss << *it;
          if (std::next(it) != where_toks.end()) {
            ss << " or ";
          }
        }
        ss << ")";
      }

      if (find_by_toks.size()) {
        if (find_toks.size() || where_toks.size()) {
          ss << " and ";
        } else {
          ss << " ";
        }

        for (auto it = find_by_toks.begin(); it != find_by_toks.end(); ++it) {
          ss << c->esc(it->first) << " = " << c->quote(it->second);
          if (std::next(it) != find_by_toks.end()) {
            ss << " and ";
          }
        }
      }
    }

    if (limit_rows) {
      ss << " limit " << limit_rows;
    }

    if (offset_rows) {
      ss << " offset " << offset_rows;
    }

    return ss.str();
  }

private:
  bool is_distinct;
  int limit_rows;
  int offset_rows;
  std::vector<val_t> find_toks;
  std::vector<std::string> where_toks;
  std::unordered_map<std::string, std::string> find_by_toks;
  std::unordered_map<std::string, std::string> order_toks;
  std::vector<std::tuple<
    std::string,  // join type
    std::string,  // joined table
    std::string,  // column name
    std::string   // column name
  >> join_toks;

  // add join for current table
  template <typename target_obj_t>
  pqxx_adapter_t &add_join(
    foreign_key_t<obj_t, val_t> (target_obj_t::*p2m)
  ) {
    std::stringstream ss_left;
    ss_left << obj_t::table.name << "." << obj_t::table.get_primary_col_name();
    auto target_col_name = target_obj_t::table.get_col_name(p2m);
    auto target_table_name = target_obj_t::table.name;
    std::stringstream ss_right;
    ss_right << target_table_name << "." << target_col_name;
    auto right_col = target_obj_t::table.get_col_name(p2m);
    join_toks.push_back(std::make_tuple(
      "inner join",
      target_obj_t::table.name,
      ss_left.str(),
      ss_right.str()
    ));
    return *this;
  }

  template<typename out_t>
  void split(const std::string &s, char delim, out_t &result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
      result.push_back(item);
    }
  }

  // allow nested joins
  template <typename other_obj_t, typename other_val_t, typename target_obj_t>
  pqxx_adapter_t add_join(
    foreign_key_t<other_obj_t, other_val_t> (target_obj_t::*p2m)
  ) {
    std::stringstream ss_left;
    ss_left << other_obj_t::table.name << "." << other_obj_t::table.get_primary_col_name();
    auto target_col_name = target_obj_t::table.get_col_name(p2m);
    auto target_table_name = target_obj_t::table.name;
    std::stringstream ss_right;
    ss_right << target_table_name << "." << target_col_name;
    auto right_col = target_obj_t::table.get_col_name(p2m);

    // check if referenced key is included in
    // join, otherwise throw error explaining
    // that nested join queries do not work
    // unless all tables link with each other
    for (const auto &item : join_toks) {
      std::vector<std::string> res;
      auto left = ss_left.str();
      split(left, '.', res);
      if (res.size() > 0 && res[0] == std::get<1>(item)) {
        join_toks.push_back(std::make_tuple(
          "inner join",
          target_obj_t::table.name,
          left,
          ss_right.str()
        ));
        return *this;
      }
    }
    std::stringstream ss;
    ss << "must join primary key for " << target_obj_t::table.name << " first";
    throw ss.str();
  }

  template <typename col_val_t>
  std::string str(const col_val_t &val) {
    std::stringstream ss;
    ss << val;
    return ss.str();
  }

};  // pqxx_adapter_t<model_t>

template <typename obj_t, typename val_t>
pqxx_adapter_t<obj_t, val_t> make_adapter(
  primary_key_t<val_t> (obj_t::*p2m)
) {
  pqxx_adapter_t<obj_t, val_t> adapter(p2m);
  return adapter;
}

// * ==================================
// * time to test ^.^
// * ==================================

class model_z_t {
public:
  const primary_key_t<int> &get_primary_key() const {
    return id;
  }

  primary_key_t<int> id;

  int age;

  static const table_t<model_z_t> table;

};

const table_t<model_z_t> model_z_t::table = {
  "model_z_t", {
    make_col("id", &model_z_t::id),
    make_col("age", &model_z_t::age)
  }
};  // model_z_t

class model_a_t {
public:
  const primary_key_t<std::string> &get_primary_key() const {
    return id;
  }

  primary_key_t<std::string> id;

  int age;

  double opacity;

  int64_t huge_num;

  foreign_key_t<model_z_t, int> foreign;

  static const table_t<model_a_t> table;

};

const table_t<model_a_t> model_a_t::table = {
  "model_a_t", {
    make_col("id", &model_a_t::id),
    make_col("opacity", &model_a_t::opacity),
    make_col("huge_num", &model_a_t::huge_num),
    make_col("age", &model_a_t::age),
    make_col("foreign", &model_a_t::foreign, &model_z_t::id)
  }
};  // model_a_t

class model_b_t {
public:
  const primary_key_t<std::string> &get_primary_key() const {
    return id;
  }

  primary_key_t<std::string> id;

  foreign_key_t<model_a_t, std::string> foreign;

  std::string name;

  static const table_t<model_b_t> table;
};

const table_t<model_b_t> model_b_t::table = {
  "model_b_t", {
    make_col("id", &model_b_t::id),
    make_col("foreign", &model_b_t::foreign, &model_a_t::id),
    make_col("name", &model_b_t::name)
  }
};  // model_b_t

class model_c_t {
public:
  const primary_key_t<std::string> &get_primary_key() const {
    return id;
  }

  primary_key_t<std::string> id;

  foreign_key_t<model_a_t, std::string> foreign;

  std::string name;

  static const table_t<model_c_t> table;
};

const table_t<model_c_t> model_c_t::table = {
  "model_c_t", {
    make_col("id", &model_c_t::id),
    make_col("foreign", &model_c_t::foreign, &model_a_t::id),
    make_col("name", &model_c_t::name)
  }
};  // model_c_t

class model_d_t {
public:
  const primary_key_t<std::string> &get_primary_key() const {
    return id;
  }

  primary_key_t<std::string> id;

  foreign_key_t<model_c_t, std::string> foreign;

  std::string name;

  static const table_t<model_d_t> table;
};

const table_t<model_d_t> model_d_t::table = {
  "model_d_t", {
    make_col("id", &model_d_t::id),
    make_col("foreign", &model_d_t::foreign, &model_c_t::id),
    make_col("name", &model_d_t::name)
  }
};  // model_c_t

FIXTURE(adapter_to_sql) {
  // find
  auto subject_find_one = make_adapter(&model_a_t::id).find("one");
  EXPECT_EQ(
    subject_find_one.to_sql(),
    "select model_a_t.* from model_a_t where (model_a_t.id = 'one')"
  );
  auto subject_find_two = make_adapter(&model_a_t::id).find("one").find("two");
  EXPECT_EQ(
    subject_find_two.to_sql(),
    "select model_a_t.* from model_a_t where (model_a_t.id = 'one' or model_a_t.id = 'two')"
  );

  // find_by
  auto subject_find_by = make_adapter(&model_a_t::id).find_by(&model_a_t::age, 1);
  EXPECT_EQ(
    subject_find_by.to_sql(),
    "select model_a_t.* from model_a_t where model_a_t.age = '1'"
  );

  // distinct limit
  auto subject_limit_distinct = make_adapter(&model_a_t::id).distinct().limit(10);
  EXPECT_EQ(
    subject_limit_distinct.to_sql(),
    "select distinct model_a_t.* from model_a_t  limit 10"
  );

  // limit offset
  auto subject_limit_offset = make_adapter(&model_a_t::id).distinct().limit(10).offset(10);
  EXPECT_EQ(
    subject_limit_offset.to_sql(),
    "select distinct model_a_t.* from model_a_t  limit 10 offset 10"
  );

  // where find_by find
  auto subject_where_find = make_adapter(&model_a_t::id)
    .where("model_a_t.id like '%keyword$%'")
    .find("123")
    .find_by(&model_a_t::age, 321)
    .limit(10);

  EXPECT_EQ(
    subject_where_find.to_sql(),
    "select model_a_t.* from model_a_t where (model_a_t.id = '123') or "
    "(model_a_t.id like '%keyword$%') and model_a_t.age = '321' limit 10"
  );
}

FIXTURE(adapter) {
  auto subject = make_adapter(&model_a_t::id);
  subject.find_by(&model_a_t::age, 321);
  subject.find("123");
  subject.find("321");
  subject.find_by("id", "123");
  subject.find_by(&model_a_t::id, "321");
  subject.find_by(&model_a_t::opacity, 0.1);
  subject.find_by(&model_a_t::huge_num, 1000000);
  subject.find_by(&model_a_t::foreign, 123);
  subject.find_by(&model_c_t::id, "420");
  subject.find_by(&model_c_t::name, "alexander");
  subject.find_by(&model_c_t::foreign, "for_321");
  subject.where("id > 0");
  subject.where("id is not null");
  subject.order("id", "asc");
  subject.order(&model_a_t::id, "asc");
  subject.order(&model_a_t::age, "asc");
  subject.order(&model_a_t::opacity, "desc");
  subject.order(&model_b_t::id);
  subject.order(&model_c_t::id);
  subject.joins(&model_b_t::foreign, &model_c_t::foreign, &model_d_t::foreign);
  //subject.write(std::cout);

  auto related_subject = make_adapter(&model_b_t::id);
  related_subject.find("123");
  related_subject.find("321");
  related_subject.find_by("id", "123");
  related_subject.find_by(&model_b_t::id, "321");
  related_subject.find_by(&model_b_t::foreign, "321");
  related_subject.find_by(&model_b_t::name, "this is name");
  related_subject.where("id > 0");
  related_subject.where("id is not null");
  related_subject.order("id", "asc");
  related_subject.order(&model_b_t::foreign, "desc");
  // related_subject.write(std::cout);
}

FIXTURE(valuable_t) {
  valuable_t<int> subject = 5;

  // conditional true
  EXPECT_TRUE(subject == 5); // ==
  EXPECT_TRUE(5 == subject);
  EXPECT_TRUE(subject != 0); // !=
  EXPECT_TRUE(0 != subject);
  EXPECT_TRUE(subject >= 5); // >=
  EXPECT_TRUE(5 >= subject);
  EXPECT_TRUE(subject <= 5); // <=
  EXPECT_TRUE(5 <= subject);
  EXPECT_TRUE(subject < 6); // <
  EXPECT_TRUE(4 < subject);
  EXPECT_TRUE(subject > 4); // >
  EXPECT_TRUE(6 > subject);

  // conditional false
  EXPECT_FALSE(subject == 0); // ==
  EXPECT_FALSE(0 == subject);
  EXPECT_FALSE(subject != 5); // !=
  EXPECT_FALSE(5 != subject);
  EXPECT_FALSE(subject >= 6); // >=
  EXPECT_FALSE(4 >= subject);
  EXPECT_FALSE(subject <= 4); // <=
  EXPECT_FALSE(6 <= subject);
  EXPECT_FALSE(subject < 5); // <
  EXPECT_FALSE(5 < subject);
  EXPECT_FALSE(subject > 5); // >
  EXPECT_FALSE(5 > subject);

  // arithmetic operator
  EXPECT_TRUE(subject + 1 == 6);
  EXPECT_TRUE(subject - 1 == 4);
  EXPECT_TRUE(subject * 2 == 10);
  EXPECT_TRUE((subject + 5) / 2 == 5);
  EXPECT_TRUE(subject % 2 == 1);
  EXPECT_TRUE(subject++ == 5);
  EXPECT_TRUE(subject == 6);
  EXPECT_TRUE(subject-- == 6);
  EXPECT_TRUE(subject == 5);

  // can't steal reference
  auto what = ++subject;
  EXPECT_TRUE(what == 6);
  EXPECT_TRUE(++what == 7);
  EXPECT_TRUE(subject == 6);
  EXPECT_TRUE(--subject == 5);

  // reassignment
  subject = 1;
  EXPECT_TRUE(subject == 1);

  // logical
  EXPECT_TRUE(1 && subject);
  EXPECT_FALSE(subject && 0);
  EXPECT_TRUE(0 || subject);
  EXPECT_TRUE(subject || 0);
  subject = 0;
  EXPECT_FALSE(subject || 0);
  EXPECT_FALSE(0 || subject);

  // operator assignment
  subject = 5;
  int another = 5;
  EXPECT_TRUE((subject += 1) == (another += 1));
  EXPECT_TRUE(subject == 6);
  EXPECT_TRUE((subject -= 1) == (another -= 1));
  EXPECT_TRUE(subject == 5);
}

FIXTURE(model_cols) {
  auto primary_col = make_col("primary_col", &model_a_t::id);
  auto foreign_col = make_col("foreign_col", &model_b_t::foreign, &model_a_t::id);
  auto regular_col = make_col("regular_col", &model_b_t::name);
  EXPECT_EQ(primary_col->is_primary_key(), true);
  EXPECT_EQ(primary_col->is_foreign_key(), false);
  EXPECT_EQ(foreign_col->is_primary_key(), false);
  EXPECT_EQ(foreign_col->is_foreign_key(), true);
  EXPECT_EQ(regular_col->is_primary_key(), false);
  EXPECT_EQ(regular_col->is_foreign_key(), false);
}

FIXTURE(table_converts_member_to_pointer_to_string) {
  EXPECT_TRUE(model_b_t::table.get_col_name(&model_b_t::foreign) == "foreign");
  EXPECT_TRUE(model_b_t::table.get_col_name(&model_b_t::name) == "name");
  EXPECT_TRUE(model_b_t::table.get_col_name(&model_b_t::id) == "id");
}

FIXTURE(create_models_and_assign) {
  model_b_t b;
  b.id = "id_b";
  b.name = "name";
  b.foreign = "id_a";
  EXPECT_TRUE(b.id == std::string("id_b"));
  EXPECT_TRUE(b.name == "name");
  EXPECT_TRUE(b.foreign == "id_a");
  EXPECT_TRUE(b.foreign->id.get().empty());
  EXPECT_TRUE(b.foreign->id->empty());
}

FIXTURE(test_assignment) {
  model_a_t instance_a;
  instance_a.id = "young self";
  instance_a.age = 23;
  std::stringstream ss1;
  model_a_t::table.write(&instance_a, ss1);
  EXPECT_TRUE(instance_a.id == "young self");
  EXPECT_TRUE(instance_a.age == 23);

  model_b_t instance_b;
  instance_b.id = "b model";
  instance_b.foreign = instance_a;
  instance_b.name = "cool";
  std::stringstream ss2;
  model_b_t::table.write(&instance_b, ss2);

  EXPECT_TRUE(instance_b.id == "b model");
  EXPECT_TRUE(instance_b.name == "cool");
  EXPECT_TRUE(instance_b.foreign == "young self");
  EXPECT_TRUE(instance_b.foreign->id == "young self");
  EXPECT_TRUE(instance_b.foreign->age == 23);
}

FIXTURE(pqxx_adapter_t) {
  model_a_t instance_a;
  instance_a.id = "young self";
  instance_a.age = 23;
  std::stringstream ss1;
  model_a_t::table.write(&instance_a, ss1);
  EXPECT_TRUE(instance_a.id == "young self");
  EXPECT_TRUE(instance_a.age == 23);

  model_b_t instance_b;
  instance_b.id = "b model";
  instance_b.foreign = instance_a;
  instance_b.name = "cool";
  std::stringstream ss2;
  model_b_t::table.write(&instance_b, ss2);
}

FIXTURE(mpark_variant_test) {
  mpark::variant<int, float> int_var(1);
  EXPECT_TRUE(mpark::get<int>(int_var) == 1);
  mpark::variant<int, float> float_var(0.5f);
  EXPECT_TRUE(mpark::get<float>(float_var) == 0.5f);
  mpark::variant<int, float, std::string> str_var("cool?");
  EXPECT_TRUE(mpark::get<std::string>(str_var) == "cool?");
}

int main(int argc, char *argv[]) {
  return test_main(argc, argv);
}