#include <hoc/main.h>
#include <iostream>

using namespace hoc;
using namespace std;

void hoc::app_t::main() {
  app_t &app = app_t::get();

  //app.log("main");

  app.on_start([&app]() {
    //app.log("start");
  });

  app.on_request([&app](const req_t &req) {
    for (auto it = req.request_headers.begin(); it != req.request_headers.end(); ++it) {
      //app.log(it->first + ": " + it->second);
    }

    for (auto it = req.response_headers.begin(); it != req.response_headers.end(); ++it) {
      //app.log(it->first + ": " + it->second);
    }

    // set some known headers
    req.response_headers["Content-Length"] = "20";
    req.response_headers["Content-Type"] = "text/html";
    req.response_headers["This-Is-Custom"] = "LOL HOW YOU DOIN";

    req.on_data([&app](const string &) {});

    req.on_end([&req]() {
      req.send_body("hello cruelish world");
    });
  });
}
