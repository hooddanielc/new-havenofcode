#include <vector>
#include <experimental/optional>
#include <hoc-test/test-helper.h>
#include <reflex/reflection.h>
#include <reflex/json-helper.h>
#include <reflex/pqxx-helper.h>
#include <hoc/db/connection.h>

using namespace hoc;
using namespace reflex;

using mirror = reflex_pack<hoc::json, pqxx::tuple>;

class fixture_reflection_t {

public:

  using ref_t = mirror::with<fixture_reflection_t>::ref_t;

  int a;

  int b;

  std::string c;

  static const ref_t reflection;

};

const fixture_reflection_t::ref_t fixture_reflection_t::reflection = fixture_reflection_t::ref_t("fixture", {
  mirror::with<fixture_reflection_t>::make_attr("a", &fixture_reflection_t::a),
  mirror::with<fixture_reflection_t>::make_attr("b", &fixture_reflection_t::b),
  mirror::with<fixture_reflection_t>::make_attr("c", &fixture_reflection_t::c)
});

/* read postgres result and read and write to json */
/******************************************/

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

  EXPECT_EQ(fixture_reflection_t::reflection.get_members().size(), size_t(3));
  EXPECT_EQ(fixture_reflection_t::reflection.get_members()[0]->name, "a");

  reflection_t<fixture_reflection_t, hoc::json, pqxx::tuple> reflex({
    mirror::with<fixture_reflection_t>::make_attr("a", &fixture_reflection_t::a),
    mirror::with<fixture_reflection_t>::make_attr("b", &fixture_reflection_t::b),
    mirror::with<fixture_reflection_t>::make_attr("c", &fixture_reflection_t::c)
  });

  int idx = 0;

  reflex.for_each([&](auto member) {
    auto name = member->name;

    switch (idx++) {
      case 0: EXPECT_EQ(name, "a"); break;
      case 1: EXPECT_EQ(name, "b"); break;
      case 2: EXPECT_EQ(name, "c"); break;
    }
  });

  auto members = reflex.get_members();
  EXPECT_EQ(members[0]->name, "a");
  EXPECT_EQ(members[1]->name, "b");
  EXPECT_EQ(members[2]->name, "c");
  fixture_reflection_t reflection;

  EXPECT_OK([&]() {
    setup_db();
    auto c = hoc::db::super_user_connection();
    pqxx::work w(*c);
    auto res = w.exec("select * from fixture");
    w.commit();
    reflex.read(&reflection, res[0]);
    EXPECT_EQ(reflection.a, 4);
    EXPECT_EQ(reflection.b, 4);
    EXPECT_EQ(reflection.c, "cman");
    hoc::json json;
    reflex.write(&reflection, json);
    EXPECT_EQ(json["a"].get<int>(), 4);
    EXPECT_EQ(json["b"].get<int>(), 4);
    EXPECT_EQ(json["c"].get<std::string>(), "cman");
    fixture_reflection_t write_from_json;
    reflex.read(&reflection, json);
    EXPECT_EQ(reflection.a, 4);
    EXPECT_EQ(reflection.b, 4);
    EXPECT_EQ(reflection.c, "cman");
  });

  EXPECT_OK([]() {
    destroy_db();
  });
}

int main(int argc, char *argv[]) {
  return test_main(argc, argv);
}
