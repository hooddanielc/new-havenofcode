#include <hoc-test/test-helper.h>

#include <hoc/json.h>

using namespace std;
using namespace dj;
using namespace hoc;

FIXTURE(create_from_string) {
  auto value = json_t::from_string("{ \"that\":\"cool\"}");
  EXPECT_EQ(value["that"] == "cool", true);
}

int main(int argc, char *argv[]) {
  return test_main(argc, argv);
}
