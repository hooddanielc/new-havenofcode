#include <hoc-test/test-helper.h>
#include <hoc/json.h>

using namespace std;
using namespace hoc;

FIXTURE(create_and_write_obj) {
  json j = {
    {"pi", 3.141},
    {"happy", true},
    {"name", "Niels"},
    {"nothing", nullptr},
    {"answer", {
      {"everything", 42}
    }},
    {"list", {1, 0, 2}},
    {"object", {
      {"currency", "USD"},
      {"value", 42.99}
    }}
  };

  std::stringstream ss;
  ss << j;
  EXPECT_GT(ss.str().size(), size_t(0));
}

FIXTURE(parse_json_and_stringify) {
  std::string str("{\"happy\":true,\"pi\":3.141}");
  auto obj = json::parse(str);
  EXPECT_EQ(obj.dump(), str);
}

FIXTURE(access) {
  json j = {
    {"pi", 3.141},
    {"happy", true},
    {"name", "Niels"},
    {"nothing", nullptr},
    {"answer", {
      {"everything", 42}
    }},
    {"list", {1, 0, 2}},
    {"object", {
      {"currency", "USD"},
      {"value", 42.99}
    }}
  };

  EXPECT_EQ(3.141, j["pi"].get<double>());
  EXPECT_TRUE(j["doesntexist"].is_null());
  EXPECT_TRUE(j.is_object());
  EXPECT_TRUE(j["list"].is_array());
  EXPECT_TRUE(j["pi"].is_number());
  EXPECT_TRUE(j["name"].is_string());
  EXPECT_TRUE(j["happy"].is_boolean());
  EXPECT_FALSE(j.is_null());
  double pi = j["pi"];
  EXPECT_EQ(3.141, pi);
}

int main(int argc, char *argv[]) {
  return test_main(argc, argv);
}
