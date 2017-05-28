#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include <ostream>

#include <hoc-active-record/valuable.h>

namespace hoc {

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
inline std::shared_ptr<col_t<obj_t, val_t>> make_col(
  const std::string &name,
  val_t (obj_t::*p2m)
) {
  return std::make_shared<col_t<obj_t, val_t>>(name, p2m);
}

template <typename obj_t, typename val_t>
inline std::shared_ptr<primary_col_t<obj_t, val_t>> make_col(
  const std::string &name,
  primary_key_t<val_t> (obj_t::*p2m)
) {
  return std::make_shared<primary_col_t<obj_t, val_t>>(name, p2m);
}

template <typename obj_t, typename val_t, typename target_obj_t>
inline std::shared_ptr<foreign_col_t<obj_t, val_t, target_obj_t>> make_col(
  const std::string &name,
  foreign_key_t<target_obj_t, val_t> (obj_t::*p2m),
  primary_key_t<val_t> (target_obj_t::*p2m_target)
) {
  return std::make_shared<foreign_col_t<obj_t, val_t, target_obj_t>>(name, p2m, p2m_target);
}

// require primary key for foreign key columns
template <typename obj_t, typename val_t, typename target_obj_t>
inline std::shared_ptr<foreign_col_t<obj_t, val_t, target_obj_t>> make_col(
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

}   // hoc