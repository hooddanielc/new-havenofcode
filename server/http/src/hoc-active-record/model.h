#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include <ostream>
#include <experimental/optional>

#include <hoc-active-record/valuable.h>
#include <hoc/db/connection.h>
#include <hoc/json.h>

namespace hoc {

class any_factory_t {
  public:

    virtual void reset() = 0;

    virtual void clear() = 0;

    void add_to_store(const std::string &factory);

};

class store_t final {
  public:
    friend class any_factory_t;

    static store_t *get() {
      static store_t store;
      return &store;
    }

    void reset() {
      for (const auto &iter: factories) {
        iter.second->reset();
      }

      if (!connection.expired()) {
        connection.reset();
      }
    }

    void clear() {
      for (const auto &iter: factories) {
        iter.second->clear();
      }
    }

    std::shared_ptr<pqxx::connection> get_connection() {
      if (connection.expired()) {
        auto con = hoc::db::anonymous_connection();
        connection = con;
        return con;
      }

      return connection.lock();
    }

    void set_connection(std::shared_ptr<pqxx::connection> connection_) {
      connection = connection_;
    }

  private:

    void add_factory(std::string name, any_factory_t *factory) {
      factories[name] = factory;
    }

    std::weak_ptr<pqxx::connection> connection;

    std::unordered_map<std::string, any_factory_t*> factories;

    store_t() = default;

    ~store_t() = default;

};

void any_factory_t::add_to_store(const std::string &name) {
  store_t::get()->add_factory(name, this);
}

template <typename obj_t, typename val_t>
class factory_t final: public any_factory_t {
public:

  std::shared_ptr<obj_t> require(const val_t &val) {
    std::lock_guard<std::mutex> lock { mutex };
    auto pair = weak_ptrs.emplace(val, std::weak_ptr<obj_t> {});
    auto &iter = pair.first;
    auto &weak_ptr = iter->second;
    auto result = weak_ptr.lock();
    if (!result) {
      try {
        result = std::make_shared<obj_t>();
        result->set_primary_key(val);
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

  std::shared_ptr<obj_t> require(const json &json) {
    auto name = obj_t::table.get_primary_col_name();
    if (json[name].is_null()) {
      throw std::runtime_error("json object does not contain primary key " + name);
    }

    val_t id = json[name].template get<val_t>();
    auto result = require(id);

    for (const auto &col: obj_t::table.get_cols()) {
      if (!json[col->name].is_null()) {
       col->read(&*result, json[col->name]);
      }
    }

    return result;
  }

  std::shared_ptr<obj_t> require(const pqxx::row &res) {
    read_table();
    std::shared_ptr<obj_t> result;

    for (int i = 0; i < int(res.size()); ++i) {
      if (
        int(res[i].table()) == postgres_oid &&
        column_order[res[i].table_column()] == std::string(obj_t::table.get_primary_col_name())
      ) {
        result = require(res[i].as<val_t>());
        break;
      }
    }

    if (!result) {
      return result;
    }

    for (int i = 0; i < int(res.size()); ++i) {
      if (int(res[i].table()) == postgres_oid) {
        if (auto col = obj_t::table.get_col(column_order[res[i].table_column()])) {
          col->read(&*result, res[i]);
        }
      }
    }

    result->changes_applied();
    return result;
  }

  virtual void clear() override {
    weak_ptrs.clear();
  }

  virtual void reset() override {
    clear();
    postgres_oid = 0;
    column_order.clear();
  }

  static factory_t *get() {
    static factory_t factory;
    return &factory;
  }

  size_t size() {
    return weak_ptrs.size();
  }

private:

  factory_t() : postgres_oid(0) {
    add_to_store(obj_t::table.name);
  }

  ~factory_t() = default;

  std::mutex mutex;

  std::unordered_map<val_t, std::weak_ptr<obj_t>> weak_ptrs;

  int postgres_oid;

  std::unordered_map<int, std::string> column_order;

  void read_table() {
    if (!postgres_oid) {
      auto c = hoc::db::super_user_connection();
      pqxx::work w(*c);
      std::stringstream ss_oid;
      ss_oid << "select '" << obj_t::table.name << "'::regclass::oid";
      auto oid = w.exec(ss_oid);
      postgres_oid = oid[0][0].as<int>();
      std::stringstream ss_column_order;
      ss_column_order << "select column_name, ordinal_position from information_schema.columns "
        "where table_schema = 'public' and table_name = '" << obj_t::table.name << "'";
      auto order = w.exec(ss_column_order);
      for (int i = 0; i < int(order.size()); ++i) {
        auto name = order[i][0].as<std::string>();
        auto num = order[i][1].as<int>();
        column_order[num - 1] = name;
      }
    }
  }

};  // factory_t<obj_t, val_t>

template <typename val_t>
class primary_key_t: public valuable_t<val_t> {
public:
  using valuable_t<val_t>::valuable_t;
  using valuable_t<val_t>::operator=;

  const val_t *operator->() const {
    return &this->val;
  }

}; // primary_key_t<obj_t>

template <typename obj_t, typename val_t>
class foreign_key_t: public valuable_t<val_t> {
public:
  using valuable_t<val_t>::valuable_t;
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

  std::shared_ptr<obj_t> get_obj() const {
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

  virtual void write(const obj_t *obj, json &json) const = 0;

  virtual void read(obj_t *obj, const pqxx::result::field &field) const = 0;

  virtual void read(obj_t *obj, const json &asdf) const = 0;

  virtual bool is_equal(obj_t *obj_a, obj_t *obj_b) const = 0;

  virtual bool is_primary_key() const {
    return false;
  }

  virtual bool is_foreign_key() const {
    return false;
  }

  virtual bool is_many() const {
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
    write_stream(obj->*p2m, strm);
  }

  template <typename regular_val_t>
  void write_stream(const regular_val_t &val, std::ostream &strm) const {
    strm << val;
  }

  template <typename optional_val_t>
  void write_stream(const std::experimental::optional<optional_val_t> &val, std::ostream &strm) const {
    if (val) {
      strm << val.value();
    } else {
      strm << "null";
    }
  }

  virtual void write(const obj_t *obj, json &json) const override {
    write_json(obj->*p2m, json);
  }

  template <typename regular_val_t>
  void write_json(const regular_val_t &val, json &json) const {
    json[any_col_t::name] = val;
  }

  template <typename optional_val_t>
  void write_json(const std::experimental::optional<optional_val_t> &val, json &json) const {
    if (val) {
      json[any_col_t::name] = val.value();
    }
  }

  virtual void read(obj_t *obj, const pqxx::result::field &field) const override {
    read_pqxx_field(obj, obj->*p2m, field);
  }

  template <typename regular_val_t>
  void read_pqxx_field(obj_t *obj, const regular_val_t &, const pqxx::result::field &field) const {
    (obj->*p2m) = field.as<regular_val_t>();
  }

  template <typename optional_val_t>
  void read_pqxx_field(obj_t *obj, const std::experimental::optional<optional_val_t> &, const pqxx::result::field &field) const {
    (obj->*p2m) = field.as<optional_val_t>();
  }

  virtual void read(obj_t *obj, const json &json) const override {
    read_json(obj, obj->*p2m, json);
  }

  template <typename regular_val_t>
  void read_json(obj_t *obj, const regular_val_t &, const json &json) const {
    (obj->*p2m) = json.get<regular_val_t>();
  }

  template <typename optional_val_t>
  void read_json(obj_t *obj, const std::experimental::optional<optional_val_t> &, const json &json) const {
    (obj->*p2m) = json.get<optional_val_t>();
  }

  virtual bool is_equal(obj_t *obj_a, obj_t *obj_b) const override {
    return (obj_a->*p2m) == (obj_b->*p2m);
  }

  virtual void read(obj_t *obj, const json &json) const override {
    obj->*p2m = json.get<val_t>();
  }

  virtual bool is_primary_key() const override {
    return false;
  }

  virtual bool is_foreign_key() const override {
    return false;
  }

  virtual bool is_many() const override {
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

  virtual void write(const obj_t *obj, json &json) const override {
    write_json((obj->*p2m).get(), json);
  }

  template <typename regular_val_t>
  void write_json(const regular_val_t &val, json &json) const {
    json[any_col_t::name] = val;
  }

  template <typename optional_val_t> // not allowed to make primary key optional
  void write_json(const std::experimental::optional<optional_val_t> &val, json &json) const = delete;

  virtual void read(obj_t *obj, const pqxx::result::field &field) const override {
    (obj->*p2m) = field.as<val_t>();
  }

  virtual void read(obj_t *obj, const json &json) const override {
    (obj->*p2m) = json.get<val_t>();
  }

  virtual bool is_equal(obj_t *obj_a, obj_t *obj_b) const override {
    return (obj_a->*p2m).get() == (obj_b->*p2m).get();
  }

  virtual void read(obj_t *obj, const json &json) const override {
    obj->*p2m = json.get<val_t>();
  }

  virtual bool is_primary_key() const override {
    return true;
  }

  virtual bool is_foreign_key() const override {
    return false;
  }

  virtual bool is_many() const override {
    return false;
  }

};  // primary_col_t<obj_t, val_t>

template <typename obj_t, typename val_t, typename target_obj_t, typename target_val_t>
class has_many_col_t;

template <typename obj_t, typename val_t, typename target_obj_t, typename target_val_t = val_t>
class foreign_col_t final: public any_col_of_t<obj_t> {
public:

  using p2m_t = foreign_key_t<target_obj_t, val_t> (obj_t::*);
  using p2m_target_t = primary_key_t<target_val_t> (target_obj_t::*);

  explicit foreign_col_t(p2m_t p2m_, p2m_target_t p2m_target_)
      : p2m(p2m_), p2m_target(p2m_target_) {}

  explicit foreign_col_t(const std::string &name_, p2m_t p2m_, p2m_target_t p2m_target_)
      : any_col_of_t<obj_t>(name_), p2m(p2m_), p2m_target(p2m_target_) {}

  virtual void write(const obj_t *obj, std::ostream &strm) const override {
    write_stream((obj->*p2m).get(), strm);
  }

  template <typename regular_val_t>
  void write_stream(const regular_val_t &val, std::ostream &strm) const {
    strm << val;
  }

  template <typename optional_val_t>
  void write_stream(const std::experimental::optional<optional_val_t> &val, std::ostream &strm) const {
    if (val) {
      strm << val.value();
    } else {
      strm << "null";
    }
  }

  virtual void write(const obj_t *obj, json &json) const override {
    write_json((obj->*p2m).get(), json);
  }

  template <typename regular_val_t>
  void write_json(const regular_val_t &val, json &json) const {
    json[any_col_t::name] = val;
  }

  template <typename optional_val_t>
  void write_json(const std::experimental::optional<optional_val_t> &val, json &json) const {
    if (val) {
      json[any_col_t::name] = val.value();
    }
  }

  virtual void read(obj_t *obj, const pqxx::result::field &field) const override {
    read_pqxx_field(obj, (obj->*p2m).get(), field);
    add_has_many_record(obj->get_primary_key(), obj);
  }

  template <typename regular_val_t>
  void read_pqxx_field(obj_t *obj, const regular_val_t &, const pqxx::result::field &field) const {
    (obj->*p2m) = field.as<regular_val_t>();
  }

  template <typename optional_val_t>
  void read_pqxx_field(obj_t *obj, const std::experimental::optional<optional_val_t> &, const pqxx::result::field &field) const {
    (obj->*p2m) = field.as<optional_val_t>();
  }

  virtual void read(obj_t *obj, const json &json) const override {
    read_json(obj, (obj->*p2m).get(), json);
    add_has_many_record(obj->get_primary_key(), obj);
  }

  template <typename regular_val_t>
  void read_json(obj_t *obj, const regular_val_t &, const json &json) const {
    (obj->*p2m) = json.get<regular_val_t>();
  }

  template <typename optional_val_t>
  void read_json(obj_t *obj, const std::experimental::optional<optional_val_t> &, const json &json) const {
    (obj->*p2m) = json.get<optional_val_t>();
  }

  virtual bool is_equal(obj_t *obj_a, obj_t *obj_b) const override {
    return (obj_a->*p2m).get() == (obj_b->*p2m).get();
  }

  virtual bool is_primary_key() const override {
    return false;
  }

  virtual bool is_foreign_key() const override {
    return true;
  }

  virtual bool is_many() const override {
    return false;
  }

  const p2m_t p2m;

  const p2m_target_t p2m_target;

private:

  template <typename primary_val_t>
  void add_has_many_record(const primary_key_t<primary_val_t> &id, obj_t *obj) const {
    using for_t = has_many_col_t<target_obj_t, val_t, obj_t, primary_val_t>*;
    for (auto &col: target_obj_t::table.get_cols()) {
      if (for_t advanced_col = dynamic_cast<for_t>(&*col)) {
        if (advanced_col->p2m_target == p2m) {
          auto &list = *((obj->*p2m).get_obj()).*(advanced_col->p2m);
          if (std::find(list.begin(), list.end(), id.get()) == list.end()) {
            list.push_back(id.get());
          }
        }
      }
    }
  }

};  // foreign_col_t<obj_t, val_t, target_obj_t>

template <typename obj_t, typename val_t, typename target_obj_t, typename target_val_t>
class has_many_col_t final: public any_col_of_t<obj_t> {
public:

  using p2m_t = std::vector<target_val_t> (obj_t::*);
  using p2m_target_t = foreign_key_t<obj_t, val_t> (target_obj_t::*);

  explicit has_many_col_t(p2m_t p2m_, p2m_target_t p2m_target_)
      : p2m(p2m_), p2m_target(p2m_target_) {}

  explicit has_many_col_t(const std::string &name_, p2m_t p2m_, p2m_target_t p2m_target_)
      : any_col_of_t<obj_t>(name_), p2m(p2m_), p2m_target(p2m_target_) {}

  virtual void write(const obj_t *obj, std::ostream &strm) const override {
    bool first = true;
    for (const auto &item: (obj->*p2m)) {
      if (!first) {
        strm << ", ";
      }
      strm << item;
      first = false;
    }
  }

  virtual void write(const obj_t *obj, json &json_obj) const override {
    json array;

    for (const auto &item: (obj->*p2m)) {
      array.emplace_back(item);
    }

    json_obj[any_col_t::name] = array;
  }

  virtual void read(obj_t *, const pqxx::result::field &) const override {}

  virtual void read(obj_t *, const json &) const override {}

  virtual bool is_equal(obj_t *, obj_t *) const override {
    return true;
  }

  virtual bool is_primary_key() const override {
    return false;
  }

  virtual bool is_foreign_key() const override {
    return false;
  }

  virtual bool is_many() const override {
    return true;
  }

  const p2m_t p2m;

  const p2m_target_t p2m_target;

};  // has_many_col_t<obj_t, val_t, target_obj_t, target_val_t>

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

template <typename obj_t, typename val_t, typename target_obj_t>
inline std::shared_ptr<foreign_col_t<obj_t, std::experimental::optional<val_t>, target_obj_t, val_t>> make_col(
  const std::string &name,
  foreign_key_t<target_obj_t, std::experimental::optional<val_t>> (obj_t::*p2m),
  primary_key_t<val_t> (target_obj_t::*p2m_target)
) {
  return std::make_shared<foreign_col_t<obj_t, std::experimental::optional<val_t>, target_obj_t, val_t>>(name, p2m, p2m_target);
}

template <typename obj_t, typename val_t, typename target_obj_t, typename target_val_t>
inline std::shared_ptr<has_many_col_t<obj_t, val_t, target_obj_t, target_val_t>> make_col(
  const std::string &name,
  std::vector<target_val_t> (obj_t::*p2m),
  foreign_key_t<obj_t, val_t> (target_obj_t::*p2m_target)
) {
  return std::make_shared<has_many_col_t<obj_t, val_t, target_obj_t, target_val_t>>(name, p2m, p2m_target);
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
        primary_col = col;
      }

      break;
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

  void write(const obj_t *obj, json &json) const {
    for (const auto &col: cols) {
      col->write(obj, json);
    }
  }

  /*
   * get_col_name(&model::member)
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

  /*
   * get_col_name(&model::foreign_key_member) optional member
   *
   * Look up the name of a foreign_col_t using a member
   * to pointer reference.
   */
  template <typename target_val_t, typename target_obj_t>
  std::string get_col_name(foreign_key_t<target_obj_t, std::experimental::optional<target_val_t>> (obj_t::*p2m)) const {
    using for_t = const foreign_col_t<obj_t, std::experimental::optional<target_val_t>, target_obj_t, target_val_t>*;
    for (const auto &col: cols) {
      if (for_t advanced_col = dynamic_cast<for_t>(&*col)) {
        if (advanced_col->p2m == p2m) {
          return advanced_col->name;
        }
      }
    }
    throw std::runtime_error("column does not exist, did you forget to make_col?");
  }

  /*
   * get_col(&model::member)
   */
  template <typename val_t>
  std::shared_ptr<any_col_of_t<obj_t>> get_col(val_t (obj_t::*p2m)) const {
    for (const auto &col: cols) {
      if (const col_t<obj_t, val_t> *cast = dynamic_cast<const col_t<obj_t, val_t>*>(&*col)) {
        if (cast->p2m == p2m) {
          return col;
        }
      }
    }
    throw std::runtime_error("column does not exist, did you forget to make_col?");
  }

  /*
   * get_col(&model::primary_key_member)
   */
  template <typename val_t>
  std::shared_ptr<any_col_of_t<obj_t>> get_col(primary_key_t<val_t> (obj_t::*p2m)) const {
    using pri_t = const primary_col_t<obj_t, val_t>*;
    for (const auto &col: cols) {
      if (pri_t cast = dynamic_cast<pri_t>(&*col)) {
        if (cast->p2m == p2m) {
          return col;
        }
      }
    }
    throw std::runtime_error("column does not exist, did you forget to make_col?");
  }

  /*
   * get_col(&model::foreign_key_member)
   */
  template <typename target_val_t, typename target_obj_t>
  std::shared_ptr<any_col_of_t<obj_t>> get_col(foreign_key_t<target_obj_t, target_val_t> (obj_t::*p2m)) const {
    using for_t = const foreign_col_t<obj_t, target_val_t, target_obj_t>*;
    for (const auto &col: cols) {
      if (for_t advanced_col = dynamic_cast<for_t>(&*col)) {
        if (advanced_col->p2m == p2m) {
          return col;
        }
      }
    }
    throw std::runtime_error("column does not exist, did you forget to make_col?");
  }

  /*
   * get_col(std::string)
   */
  std::shared_ptr<any_col_of_t<obj_t>> get_col(const std::string &col_name) const {
    for (const auto &col: cols) {
      if (col->name == col_name) {
        return col;
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
  std::shared_ptr<any_col_of_t<obj_t>> primary_col;
  std::vector<std::shared_ptr<any_col_of_t<obj_t>>> cols;

};  // table_t<obj_t>

template <typename obj_t, typename val_t> class sql_adapter_t;

template <typename obj_t>
class model_t {
public:
  using array_t = std::vector<std::shared_ptr<obj_t>>;

  /*
   * get_primary_key()
   *
   * returns primary key
   */
  auto get_primary_key() const {
    return static_cast<const obj_t*>(this)->id;
  }

  /*
   * get_primary_mpt()
   *
   * return member to pointer for primary key
   */
  static auto get_primary_mpt() {
    return &obj_t::id;
  }

  template <typename val_t>
  void set_primary_key(const val_t &val) {
    static_cast<obj_t*>(this)->id = val;
  }

  template <typename val_t>
  void set_primary_key(const std::experimental::optional<val_t> &val) {
    if (val) {
      static_cast<obj_t*>(this)->id = val.value();
    }
  }

  /*
   * set(&model_t::property, val_t)
   */
  template <typename val_t>
  void set(val_t (obj_t::*p2m), const val_t &val) {
    static_cast<obj_t*>(this)->*p2m = val;
  }

  template <typename val_t, typename target_t>
  void set(foreign_key_t<target_t, val_t> (obj_t::*p2m), const val_t &val) {
    static_cast<obj_t*>(this)->*p2m = val;
  }

  template <typename val_t>
  void set(primary_key_t<val_t> (obj_t::*p2m), const val_t &val) = delete;

  /*
   * is_dirty()
   */
  bool is_dirty() {
    for (const auto &col: obj_t::table.get_cols()) {
      if (!col->is_equal(static_cast<obj_t*>(original.get()), static_cast<obj_t*>(this))) {
        return true;
      }
    }

    return false;
  }

  /*
   * changes_applied()
   *
   * saves current changes as original state
   */
  void changes_applied() {
    original = std::make_shared<obj_t>(*(static_cast<obj_t*>(this)));
  }

  /*
   * get_dirty_attributes()
   */
  std::vector<std::string> get_dirty_attributes() {
    std::vector<std::string> attrs;
    for (const auto &col: get_dirty_cols()) {
      attrs.push_back(col->name);
    }
    return attrs;
  }

  /*
   * save()
   */
  void save(std::shared_ptr<pqxx::connection> c = store_t::get()->get_connection()) {
    pqxx::work w(*c);
    save(w, true);
  }

  void save(pqxx::work &w) {
    save(w, false); // do not commit immediately if user supplied transaction object
  }

  void save(pqxx::work &w, bool commit_now) {
    if (is_dirty()) {
      std::stringstream ss;
      ss << "update " << obj_t::table.name << " set ";

      bool first = true;
      for (const auto &col: get_dirty_cols()) {
        if (!first) {
          ss << ", ";
        }
        std::stringstream ss_name;
        col->write(static_cast<obj_t*>(this), ss_name);
        ss << w.quote_name(col->name) << " = " << w.quote(ss_name);
        first = false;
      }

      std::stringstream ss_primary_val;
      ss_primary_val << get_primary_key();
      ss << " where " << w.quote_name(obj_t::table.get_primary_col_name()) << " = ";
      ss << w.quote(ss_primary_val);
      w.exec(ss);

      if (commit_now) {
        w.commit();
      }

      changes_applied();
    }
  }

  /*
   * reload();
   */
  void reload(std::shared_ptr<pqxx::connection> c = store_t::get()->get_connection()) {
    pqxx::work w(*c);
    reload(w);
  }

  void reload(pqxx::work &w) {
    reload(w, get_primary_key());
  }

  template <typename primary_val_t>
  void reload(pqxx::work &w, const primary_key_t<primary_val_t> &) {
    std::stringstream ss_primary_val;
    ss_primary_val << get_primary_key();
    std::stringstream ss;
    ss << "select * from " << obj_t::table.name << " where "
       << w.quote_name(obj_t::table.get_primary_col_name()) << " = "
       << w.quote(ss_primary_val);
    auto res = w.exec(ss);
    if (res.size() != 1) {
      ss << " returned " << res.size() << " rows";
      throw std::runtime_error(ss.str());
    }

    factory_t<obj_t, primary_val_t>::get()->require(res[0]);
  }

  /*
   * to_json()
   */
  json to_json() {
    json result;
    obj_t::table.write(static_cast<obj_t*>(this), result);
    return result;
  }

  /*
   * find()
   */
  template <typename primary_val_t>
  static std::shared_ptr<obj_t> find(
    const primary_val_t &val,
    std::shared_ptr<pqxx::connection> c = store_t::get()->get_connection()
  ) {
    pqxx::work w(*c);
    return find(val, w);
  }

  template <typename primary_val_t>
  static std::shared_ptr<obj_t> find(
    const primary_val_t &val,
    pqxx::work &w
  ) {
    using primary_t = decltype(std::declval<obj_t>().get_primary_key().get());
    auto result = factory_t<obj_t, primary_t>::get()->require(static_cast<primary_t>(val));
    result->reload(w);
    return result;
  }

  /*
   * find_by(key, val)
   *
   * returns sql adapter that can return rows of data
   */
  template <typename key_val_t, typename val_t>
  static auto find_by(
    const key_val_t &key,
    const val_t &val
  ) {
    auto adapter = make_sql_adapter(get_primary_mpt());
    return adapter.find_by(key, val);
  }

  /*
   * where(string)
   */
  static auto where(const std::string &str) {
    auto adapter = make_sql_adapter(get_primary_mpt());
    return adapter.where(str);
  }

private:

  std::shared_ptr<obj_t> original;

  std::vector<std::shared_ptr<any_col_of_t<obj_t>>> get_dirty_cols() {
    std::vector<std::shared_ptr<any_col_of_t<obj_t>>> result;
    for (const auto &col: obj_t::table.get_cols()) {
      if (!col->is_equal(static_cast<obj_t*>(original.get()), static_cast<obj_t*>(this))) {
        result.push_back(col);
      }
    }
    return result;
  }

};

}   // hoc
