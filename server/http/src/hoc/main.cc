#include <hoc/main.h>
#include <iostream>

using namespace hoc;
using namespace std;

void hoc::app_t::main() {
  app_t &app = app_t::get();

  app.log("main");

  app.on_start([&app]() {
    app.log("start");
  });

  app.on_request([&app](const req_t &req) {
    // print the headers
    app.log("Print Headers");
    app.log("=============");

    for(auto it = req.headers.begin(); it != req.headers.end(); ++it) {
      app.log(it->first + ": " + it->second);
      // iterator->first = key
      // iterator->second = value
      // Repeat if you also want to iterate through the second map.
    }
    req.on_data([&app](const string &data) {
      app.log("on data event");
      app.log(data);
    });

    app.log("on request");
  });
}
