#include <hoc/main.h>
#include <iostream>

using namespace hoc;
using namespace std;

void hoc::app_t::main() {
  app_t &app = app_t::get();

  app.on_start([]() {
    assign_routes();
  });

  app.on_request([&app](const req_t &req) {
    route_request(req);

    req.on_data([&app](const string &) {
      // get the request data located in body
      // of http request
    });

    req.on_end([&req, &app]() {
      for (auto it = req.request_headers.begin(); it != req.request_headers.end(); ++it) {
        // print headers
        app.log(it->first + ": " + it->second);
      }

      // set some known headers
      string hello("<html>hello <a href=\"/there\">there</a>, how are you<html/>");
      req.set_status(200);
      req.send_header("Content-Type", "text/html");
      req.set_content_length(hello.size());
      req.send_body(hello);
    });
  });
}
