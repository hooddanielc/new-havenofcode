#include <hoc-test/test-helper.h>

#include <vector>
#include <functional>
#include <memory>
#include <ostream>
#include <map>
#include <unordered_map>
#include <mutex>

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

template <typename obj_t>
class table_t final {
public:

  const std::string name;

  constexpr table_t(const std::string &&name_, std::vector<std::shared_ptr<any_col_of_t<obj_t>>> &&cols_)
      : name(std::move(name_)), cols(std::move(cols_)) {
    assert(!singleton);
    singleton = this;
  }

  ~table_t() {
    singleton = nullptr;
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
    std::string empty;
    return empty;
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
    std::string empty;
    return empty;
  }

  /*
   * get_col_name(&model::foreign_key_member)
   *
   * Look up the name of a foreign_col_t using a member
   * to pointer reference.
   */
  template <typename val_t, typename target_obj_t>
  std::string get_col_name(foreign_key_t<target_obj_t, val_t> (obj_t::*p2m)) const {
    using for_t = const foreign_col_t<obj_t, val_t, target_obj_t>*;
    for (const auto &col: cols) {
      if (for_t advanced_col = dynamic_cast<for_t>(&*col)) {
        if (advanced_col->p2m == p2m) {
          return advanced_col->name;
        }
      }
    }
    std::string empty;
    return empty;
  }

  std::vector<std::shared_ptr<any_col_of_t<obj_t>>> get_cols() const {
    return cols;
  }

private:

  table_t *singleton;
  std::vector<std::shared_ptr<any_col_of_t<obj_t>>> cols;

};  // table_t<obj_t>

template <typename model_t>
class pqxx_adapter_t {
public:

  // todo
  template <typename primary_val_t>
  pqxx_adapter_t &find(const primary_val_t &val) {
    return *this;
  }

  // todo
  template <typename obj_t, typename val_t>
  pqxx_adapter_t & find_by(
    val_t (obj_t::*)
  ) {
    return *this;
  }

  // todo where

private:
  bool is_distinct;
  std::vector<std::string> where;
  std::vector<std::string> order;
};  // pqxx_adapter_t<model_t>

// * ==================================
// * time to test ^.^
// * ==================================

class model_a_t {
public:
  const primary_key_t<std::string> &get_primary_key() const {
    return id;
  }

  primary_key_t<std::string> id;

  int age;

  static const table_t<model_a_t> table;

};

const table_t<model_a_t> model_a_t::table = {
  "model_a_t", {
    make_col("id", &model_a_t::id),
    make_col("age", &model_a_t::age)
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
  pqxx_adapter_t<model_a_t> adapter;

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

int main(int argc, char *argv[]) {
  return test_main(argc, argv);
}