#include <hoc/main.h>
#include <hoc/request.h>

using namespace hoc;
using namespace std;

void hoc::app_t::main() {
  app_t &app = app_t::get();

  app.on_start([]() {
    // intialize http client
    request_t::init();
    // initialize aws c++ sdk
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    // assign route handlers
    assign_routes();
  });

  app.on_exit([]() {
    // shutdown aws
    Aws::SDKOptions options;
    Aws::ShutdownAPI(options);
  });

  app.on_request([](req_t &req) {
    route_request(req);
  });
}
