#include <lick/lick.h>
#include <hoc/route.h>

using namespace hoc;
using namespace std;

class fixt: public route_t<string> {
  public:
    fixt() : route_t<string>("/something/:id") {}
};

FIXTURE(initial_state) {
  fixt cool;
  cool.match("asdf");
}

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}