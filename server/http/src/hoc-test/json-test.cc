#include <hoc/lick.h>
#include <hoc/json.h>
#include <iostream>

using namespace std;
using namespace dj;

FIXTURE(create_from_string) {
  auto value = json_t::from_string("{ \"that\":\"cool\"}");
  EXPECT_EQ(value["that"] == "cool", true);
}

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}