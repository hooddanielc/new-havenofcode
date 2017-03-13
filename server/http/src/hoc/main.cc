#include <hoc/main.h>

using namespace hoc;
using namespace std;

void hoc::app_t::main() {
  app_t &app = app_t::get();

  app.on_start([]() {
    assign_routes();
  });

  app.on_request([](req_t &req) {
    route_request(req);
  });
}
