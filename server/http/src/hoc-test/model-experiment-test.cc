#include <hoc-test/test-helper.h>
#include <unordered_map>
#include <hoc/json.h>

#include <reflex/reflection.h>
#include <reflex/json-helper.h>
#include <reflex/pqxx-helper.h>

using namespace std;
using namespace hoc;
using namespace reflex;

using mirror = reflex_pack<hoc::json, pqxx::tuple>;

template <typename val_t>
class container_t {

public:

  container_t(): is_set(false) {}

  container_t(val_t &&val_): is_set(true), val(std::move(val_)) {}

  bool has_value() {
    return is_set;
  }

  val_t &value_or(const val_t &val_) {
    if (has_value()) {
      return val;
    }

    return val_;
  }

  void reset() {
    if (is_set) {
      val.~val_t();
      is_set = false;
    }
  }

  val_t &value() {
    if (!is_set) {
      throw std::runtime_error("can't call get on unintialized container_t");
    }

    return val;
  }

  const val_t &value() const {
    if (!is_set) {
      throw std::runtime_error("can't call get on unintialized container_t");
    }

    return val;
  }

  void swap(val_t &&val_) {
    reset();
    val = std::move(val_);
    is_set = true;
  }

private:

  bool is_set;

  val_t val;

};

template <typename obj_t, typename val_t>
class container_of_obj_t: public container_t<val_t> {

public:

  using container_t<val_t>::container_t;

private:

};

template <typename obj_t, typename val_t>
class primary_key_t: public container_of_obj_t<obj_t, val_t> {

public:

  using container_of_obj_t<obj_t, val_t>::container_of_obj_t;

  primary_key_t() {
    static_assert(std::is_same<
      primary_key_t<obj_t, val_t> (obj_t::*),
      decltype(&obj_t::id)
    >::value, "primary key name must be 'id'");
  }

};

template <typename obj_t, typename linked_obj_t, typename val_t>
class foreign_key_t: public container_of_obj_t<obj_t, val_t> {

public:

  using container_of_obj_t<obj_t, val_t>::container_of_obj_t;

  foreign_key_t() {
    static_assert(std::is_same<
      primary_key_t<linked_obj_t, val_t> (linked_obj_t::*),
      decltype(&linked_obj_t::id)
    >::value, "foreign key must be the same type as linked_obj_t::id");
  }

};

template <typename obj_t, typename val_t>
struct member_helper_t<obj_t, container_t<val_t>, hoc::json> {

  static constexpr bool is_specialized { true };

  static void read(const std::string &name, container_t<val_t> &val, const hoc::json &from) {
    member_helper_t<obj_t, val_t, hoc::json>::read(name, val.value(), from);
  }

  static void write(const std::string &name, const container_t<val_t> &val, hoc::json &from) {
    member_helper_t<obj_t, val_t, hoc::json>::write(name, val.value(), from);
  }

};  // member_helper_t<obj_t, val_t, hoc::json>

template <typename obj_t, typename val_t>
struct member_helper_t<obj_t, container_of_obj_t<obj_t, val_t>, hoc::json>:
public member_helper_t<obj_t, container_t<val_t>, hoc::json> {};

template <typename obj_t, typename val_t>
struct member_helper_t<obj_t, primary_key_t<obj_t, val_t>, hoc::json>:
public member_helper_t<obj_t, container_t<val_t>, hoc::json> {};

template <typename obj_t, typename val_t, typename linked_obj_t>
struct member_helper_t<obj_t, foreign_key_t<obj_t, linked_obj_t, val_t>, hoc::json, linked_obj_t, primary_key_t<linked_obj_t, val_t>>:
public member_helper_t<obj_t, container_t<val_t>, hoc::json> {};

template <typename obj_t, typename val_t>
struct member_helper_t<obj_t, container_t<val_t>, pqxx::tuple> {

  static constexpr bool is_specialized { true };

  static void read(const std::string &name, container_t<val_t> &val, const pqxx::tuple &from) {
    member_helper_t<obj_t, val_t, pqxx::tuple>::read(name, val.value(), from);
  }

  static void write(const std::string &name, const container_t<val_t> &val, pqxx::tuple &from) {
    member_helper_t<obj_t, val_t, pqxx::tuple>::write(name, val.value(), from);
  }

};  // member_helper_t<obj_t, val_t, pqxx::tuple>

template <typename obj_t, typename val_t>
struct member_helper_t<obj_t, container_of_obj_t<obj_t, val_t>, pqxx::tuple>:
public member_helper_t<obj_t, container_t<val_t>, pqxx::tuple> {};

template <typename obj_t, typename val_t>
struct member_helper_t<obj_t, primary_key_t<obj_t, val_t>, pqxx::tuple>:
public member_helper_t<obj_t, container_t<val_t>, pqxx::tuple> {};

template <typename obj_t, typename val_t, typename linked_obj_t>
struct member_helper_t<obj_t, foreign_key_t<obj_t, linked_obj_t, val_t>, pqxx::tuple, linked_obj_t, primary_key_t<linked_obj_t, val_t>>:
public member_helper_t<obj_t, container_t<val_t>, pqxx::tuple> {};

class model_t {

public:

  model_t() {}

};

class model_a_t: public model_t {

public:

  using mirror = mirror::with<model_a_t>;

  using ref_t = mirror::ref_t;

  static ref_t reflection;

  primary_key_t<model_a_t, std::string> id;

  int a;

  int b;

  std::string c;

};

model_a_t::ref_t model_a_t::reflection = model_a_t::ref_t("model_a", {
  mirror::with<model_a_t>::make_attr("id", &model_a_t::id),
  mirror::with<model_a_t>::make_attr("a", &model_a_t::a),
  mirror::with<model_a_t>::make_attr("b", &model_a_t::b),
  mirror::with<model_a_t>::make_attr("c", &model_a_t::c)
});

class model_b_t: public model_t {

public:

  using mirror = mirror::with<model_b_t>;

  using ref_t = mirror::ref_t;

  static ref_t reflection;

  primary_key_t<model_b_t, std::string> id;

  foreign_key_t<model_b_t, model_a_t, std::string> buddy;

  int a;

  int b;

  std::string c;

};

model_b_t::ref_t model_b_t::reflection = model_b_t::ref_t("model_b", {
  model_b_t::mirror::make_attr("id", &model_b_t::id),
  model_b_t::mirror::make_linked_attr("buddy", &model_b_t::buddy, &model_a_t::id),
  model_b_t::mirror::make_attr("a", &model_b_t::a),
  model_b_t::mirror::make_attr("b", &model_b_t::b),
  model_b_t::mirror::make_attr("c", &model_b_t::c)
});

FIXTURE(some_container) {
  container_t<int> container_uninitialized;

  EXPECT_FAIL([&]() {
    container_uninitialized.value() = 1;
  });

  container_t<int> container_initialized(1);
  EXPECT_EQ(container_initialized.value(), 1);
  container_initialized.value() = 3;
  EXPECT_EQ(container_initialized.value(), 3);
  container_initialized.reset();
}

FIXTURE(with_primary_key) {
  EXPECT_OK([]() {
    model_a_t model;
    model.id.swap("id");
    model.a = 1;
    model.b = 2;
    model.c = "string";

    hoc::json json_obj;
    model_a_t::reflection.write(&model, json_obj);

    EXPECT_EQ(json_obj["id"].get<std::string>(), "id");
    EXPECT_EQ(json_obj["a"].get<int>(), 1);
    EXPECT_EQ(json_obj["b"].get<int>(), 2);
    EXPECT_EQ(json_obj["c"].get<std::string>(), "string");
  });

  EXPECT_OK([]() {
    model_b_t model;
    model.id.swap("id");
    model.buddy.swap("buddy");
    model.a = 1;
    model.b = 2;
    model.c = "string";

    hoc::json json_obj;
    model_b_t::reflection.write(&model, json_obj);

    EXPECT_EQ(json_obj["id"].get<std::string>(), "id");
    EXPECT_EQ(json_obj["buddy"].get<std::string>(), "buddy");
    EXPECT_EQ(json_obj["a"].get<int>(), 1);
    EXPECT_EQ(json_obj["b"].get<int>(), 2);
    EXPECT_EQ(json_obj["c"].get<std::string>(), "string");
  });
}

FIXTURE(ok) {
  model_a_t fixture;
}

int main(int argc, char *argv[]) {
  return test_main(argc, argv);
}
