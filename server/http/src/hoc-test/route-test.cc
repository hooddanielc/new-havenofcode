#include <lick/lick.h>
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

FIXTURE(initial_state) {
  fixt simple;
  auto simple_match = simple.match("/something/1234");
  EXPECT_EQ(simple_match.pass, true);
  EXPECT_EQ(simple_match.params[0], "1234");

  auto simple_match_trailing = simple.match("/something/1234/a");
  EXPECT_EQ(simple_match_trailing.pass, false);

  auto trailing_slash_simple_match = simple.match("/something/1234/");
  EXPECT_EQ(trailing_slash_simple_match.pass, true);

  fixt_with_multiple multiple;
  auto multiple_match = multiple.match("/something/1234/4321");
  EXPECT_EQ(multiple_match.pass, true);
  EXPECT_EQ(multiple_match.params[0], "1234");
  EXPECT_EQ(multiple_match.params[1], "4321");

  auto trailing_slash_match = multiple.match("/something/1234/4321/");
  EXPECT_EQ(trailing_slash_match.pass, true);
  EXPECT_EQ(trailing_slash_match.params[0], "1234");
  EXPECT_EQ(trailing_slash_match.params[1], "4321");

  auto trailing_path_match = multiple.match("/something/1234/4321/s");
  EXPECT_EQ(trailing_path_match.pass, false);
}

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}