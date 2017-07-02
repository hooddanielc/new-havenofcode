#include <vector>
#include <experimental/optional>
#include <hoc-test/test-helper.h>
#include <reflection/reflection.h>
#include <reflection/json-serializer.h>
#include <reflection/pqxx-serializer.h>
#include <hoc/db/connection.h>

using namespace hoc;
using namespace std;

class empty_t {};

FIXTURE(member_t) {
  any_member_of_obj_t<std::string> test_one("test_one");
  any_member_of_obj_t<int> test_two("test_two");
  any_member_of_obj_t<char> test_three("test_three");
  any_member_of_obj_t<empty_t> test_four("test_four");

  vector<any_member_t> members({
    test_one,
    test_two,
    test_three,
    test_four
  });

  EXPECT_EQ(members[0].name, "test_one");
  EXPECT_EQ(members[1].name, "test_two");
  EXPECT_EQ(members[2].name, "test_three");
  EXPECT_EQ(members[3].name, "test_four");
}

class fixture_reflection_t {
public:
  int a;
  int b;
  std::string c;

  static std::string name;
};

std::string fixture_reflection_t::name = "fixture";

const char *create_tables = R"(
create table if not exists fixture (
  a          int primary key default 1 not null,
  b          int default 1 not null,
  c          text not null
);

insert into fixture (a, b, c) values (4, 5, 'cman');
)";

const char *drop_tables = R"(
drop table if exists fixture;
)";

void setup_db() {
  EXPECT_OK([]() {
    auto c = hoc::db::super_user_connection();
    pqxx::work w(*c);
    w.exec(create_tables);
    w.commit();
  });
}

void destroy_db() {
  EXPECT_OK([]() {
    auto c = hoc::db::super_user_connection();
    pqxx::work w(*c);
    w.exec(drop_tables);
    w.commit();
  });
}

FIXTURE(reflection_t) {
  reflection_t<fixture_reflection_t> reflex({
    make_attr("a", &fixture_reflection_t::a),
    make_attr("b", &fixture_reflection_t::b)
  });

  reflex.add_attribute("c", &fixture_reflection_t::c);

  int idx = 0;

  reflex.for_each([&](auto member) {
    auto name = member->get_name();

    switch (idx++) {
      case 0: EXPECT_EQ(name, "a"); break;
      case 1: EXPECT_EQ(name, "b"); break;
      case 2: EXPECT_EQ(name, "c"); break;
    }
  });

  auto members = reflex.get_members();
  EXPECT_EQ(members[0]->get_name(), "a");
  EXPECT_EQ(members[1]->get_name(), "b");
  EXPECT_EQ(members[2]->get_name(), "c");
}

class fixture_member_string_t: public member_t<fixture_reflection_t, std::string, json_serializer_t<fixture_reflection_t, std::string>> {
  using member_t::member_t;
};

class fixture_member_int_t: public member_t<fixture_reflection_t, int, json_serializer_t<fixture_reflection_t, int>> {
  using member_t::member_t;
};

FIXTURE(member_serializer) {
  fixture_reflection_t fixture;
  fixture.a = 1;
  fixture.b = 2;
  fixture.c = "c";
  fixture_member_string_t member_c("c", &fixture_reflection_t::c);
  fixture_member_int_t member_a("a", &fixture_reflection_t::a);

  json json_obj;
  member_a.write(&fixture, json_obj);
  member_c.write(&fixture, json_obj);

  fixture_reflection_t fixture_read;
  member_a.read(&fixture_read, json_obj);
  member_c.read(&fixture_read, json_obj);

  EXPECT_EQ(json_obj["c"], "c");
  EXPECT_EQ(fixture_read.a, 1);
  EXPECT_EQ(fixture_read.c, "c");
}

template <typename obj_t, typename val_t>
inline auto make_custom_attribute(const std::string &name, val_t (obj_t::*ptr)) {
  return std::make_unique<member_t<
    obj_t,
    val_t,
    json_serializer_t<obj_t, val_t>,
    pqxx_serializer_t<obj_t, val_t>
  >>(name, ptr);
}

FIXTURE(apply_multiple_serializers_to_reflection) {
  auto member1 = make_custom_attribute("a", &fixture_reflection_t::a);
  auto member2 = make_custom_attribute("b", &fixture_reflection_t::b);
  auto member3 = make_custom_attribute("c", &fixture_reflection_t::c);
}

class fixture_member_pqxx_string_t: public member_t<fixture_reflection_t, std::string, pqxx_serializer_t<fixture_reflection_t, std::string>> {
  using member_t::member_t;
};

class fixture_member_pqxx_int_t: public member_t<fixture_reflection_t, int, pqxx_serializer_t<fixture_reflection_t, int>> {
  using member_t::member_t;
};

FIXTURE(pqxx_serializer) {
  EXPECT_OK([]() {
    setup_db();

    fixture_member_pqxx_int_t member_a("a", &fixture_reflection_t::a);
    fixture_member_pqxx_string_t member_c("c", &fixture_reflection_t::c);
    fixture_reflection_t fixture;
    fixture.a = 1;
    fixture.b = 2;
    fixture.c = "c";

    auto c = hoc::db::super_user_connection();
    pqxx::work w(*c);
    auto res = w.exec("select * from fixture");
    w.commit();

    member_a.read(&fixture, res[0]);
    member_c.read(&fixture, res[0]);

    EXPECT_EQ(fixture.a, 4);
    EXPECT_EQ(fixture.c, "cman");
  });

  EXPECT_OK([]() {
    destroy_db();
  });
}

class fixture_reflection_optional_t {

public:

  std::experimental::optional<int> a;

  std::experimental::optional<std::string> b;

};

class fixture_member_optional_json_int_t: public member_t<fixture_reflection_optional_t, std::experimental::optional<int>, json_serializer_t<fixture_reflection_optional_t, std::experimental::optional<int>>> {
  using member_t::member_t;
};

class fixture_member_optional_json_string_t: public member_t<fixture_reflection_optional_t, std::experimental::optional<std::string>, json_serializer_t<fixture_reflection_optional_t, std::experimental::optional<std::string>>> {
  using member_t::member_t;
};

FIXTURE(specialization_for_optional) {
  fixture_member_optional_json_int_t member_a("a", &fixture_reflection_optional_t::a);
  fixture_member_optional_json_string_t member_b("b", &fixture_reflection_optional_t::b);
  fixture_reflection_optional_t fixture;
  fixture.a = 1;
  fixture.b = "b";
  json json_obj;
  json_obj["a"] = 1;

  member_a.read(&fixture, json_obj);
  member_b.read(&fixture, json_obj);

}

int main(int argc, char *argv[]) {
  return test_main(argc, argv);
}
