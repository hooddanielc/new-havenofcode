#include <hoc-test/test-helper.h>

#include <hoc/route.h>

using namespace hoc;
using namespace std;

class fixt: public route_t<string> {
  public:
    fixt() : route_t<string>("/something/:id") {}
};

class fixt_with_multiple: public route_t<string> {
  public:
    fixt_with_multiple() : route_t<string>("/something/:id/:other") {}
};

class fixt_no_param: public route_t<string> {
  public:
    fixt_no_param() : route_t<string>("/api/login") {}
};

FIXTURE(simple_match) {
  fixt simple;
  auto simple_match = simple.match("/something/1234");
  EXPECT_EQ(simple_match.pass, true);
  EXPECT_EQ(simple_match.params[0], "1234");
}

FIXTURE(trailing_unmatched) {
  fixt simple;
  auto simple_match_trailing = simple.match("/something/1234/a");
  EXPECT_EQ(simple_match_trailing.pass, false);
}

FIXTURE(trailing_slash) {
  fixt simple;
  auto trailing_slash_simple_match = simple.match("/something/1234/");
  EXPECT_EQ(trailing_slash_simple_match.pass, true);
}

FIXTURE(mutliple_params) {
  fixt_with_multiple multiple;
  auto multiple_match = multiple.match("/something/1234/4321");
  EXPECT_EQ(multiple_match.pass, true);
  EXPECT_EQ(multiple_match.params[0], "1234");
  EXPECT_EQ(multiple_match.params[1], "4321");
}

FIXTURE(multiple_params_trailing_slash) {
  fixt_with_multiple multiple;
  auto trailing_slash_match = multiple.match("/something/1234/4321/");
  EXPECT_EQ(trailing_slash_match.pass, true);
  EXPECT_EQ(trailing_slash_match.params[0], "1234");
  EXPECT_EQ(trailing_slash_match.params[1], "4321");
}

FIXTURE(multiple_params_trailing_val) {
  fixt_with_multiple multiple;
  auto trailing_path_match = multiple.match("/something/1234/4321/s");
  EXPECT_EQ(trailing_path_match.pass, false);
}

FIXTURE(no_params) {
  fixt_no_param noparam;
  auto no_params = noparam.match("/api/login");
  EXPECT_EQ(no_params.pass, true);
}

int main(int argc, char *argv[]) {
  return test_main(argc, argv);
}
