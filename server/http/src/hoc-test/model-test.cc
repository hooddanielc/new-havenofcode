#include <lick/lick.h>
#include <vector>
#include <functional>
#include <memory>
#include <ostream>
#include <map>

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

protected:

  any_col_of_t() = default;

};  // any_col_of_t<obj_t>

template <typename obj_t, typename val_t>
class col_t final: public any_col_of_t<obj_t> {
public:

  using p2m_t = val_t (obj_t::*);

  explicit col_t(p2m_t p2m_)
      : p2m(p2m_) {}

  explicit col_t(const std::string &name_, p2m_t p2m_)
      : any_col_of_t<obj_t>(name_), p2m(p2m_) {}

  virtual void write(const obj_t *obj, std::ostream &strm) const override {
    strm << (obj->*p2m);
  }

private:

  p2m_t p2m;

};  // col_t<obj_t, val_t>

template <typename obj_t, typename type_t>
class primary_key_t {
public:
  type_t ref;
  primary_key_t &operator=(const type_t &other) {
    ref = other;
    return *this;
  }
};

template <typename obj_t, typename type_t>
std::ostream &operator<<(std::ostream &os, const primary_key_t<obj_t, type_t> &that) {
  os << "primary key for " << obj_t::table.name << " : " << that.ref;
  return os;
}

template <typename obj_t, typename type_t>
class foreign_key_of_t {
public:
  virtual foreign_key_of_t &operator=(const type_t &other) {
    ref = other;
    return *this;
  }

  virtual type_t get_ref() const {
    return ref;
  }

private:
  type_t ref;
};

template <typename obj_t, typename type_t, type_t obj_t::*ptr>
class foreign_key_t: public foreign_key_of_t<obj_t, type_t> {
public:
  virtual foreign_key_t &operator=(const type_t &other) {
    ref = other;
    return *this;
  }

  virtual foreign_key_t &operator=(const obj_t &other) {
    ref = other.*ptr;
    return *this;
  }

  virtual type_t get_ref() const {
    return ref;
  }

private:
  type_t ref;
};

template <typename obj_t, typename type_t>
std::ostream &operator<<(std::ostream &os, const foreign_key_of_t<obj_t, type_t> &key) {
  os << "foreign key references " << obj_t::table.name << " : " << key.get_ref();
  return os;
}

template <typename obj_t, typename val_t>
std::shared_ptr<col_t<obj_t, val_t>> make_col(
  const std::string &name,
  val_t (obj_t::*p2m)
) {
  return std::make_shared<col_t<obj_t, val_t>>(name, p2m);
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
    bool first = true;
    for (const auto &col: cols) {
      strm << (first ? "{ " : ", ") << col->name << ": ";
      col->write(obj, strm);
      first = false;
    }
    strm << (first ? "{}" : " }");
  }

private:

  table_t *singleton;
  std::vector<std::shared_ptr<any_col_of_t<obj_t>>> cols;

};  // table_t<obj_t>

class model_a_t {
public:
  std::string id;
  int age;
  static const table_t<model_a_t> table;
};

const table_t<model_a_t> model_a_t::table = {
  "model_a_t", {
    make_col("id", &model_a_t::id),
    make_col("age", &model_a_t::age)
  }
};

class model_b_t {
public:
  primary_key_t<model_b_t, std::string> id;
  foreign_key_t<model_a_t, std::string, &model_a_t::id> foreign;
  std::string name;
  static const table_t<model_b_t> table;
};

const table_t<model_b_t> model_b_t::table = {
  "model_b_t", {
    make_col("id", &model_b_t::id),
    make_col("foreign", &model_b_t::foreign),
    make_col("name", &model_b_t::name)
  }
};

FIXTURE(test_names) {
  model_a_t instance_a;
  instance_a.id = "young self";
  instance_a.age = 23;
  model_a_t::table.write(&instance_a, std::cout);

  model_b_t instance_b;
  instance_b.id = "b model";
  instance_b.foreign = instance_a;
  instance_b.name = "cool";
  model_b_t::table.write(&instance_b, std::cout);
}

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}
