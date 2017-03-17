#pragma once

#include <hoc/route.h>

namespace hoc {
  template<typename T>
  class login_route_t : public route_t<T> {
    public:
      login_route_t() : route_t<T>("/api/login") {}

      void get(T &req, const url_match_result_t &) override {
        app_t &app = app_t::get();

        req.on_end([&req, &app]() {
          // set some known headers
          std::string hello("<html>hello <a href=\"/there\">there</a>, how are you<html/>");
          req.set_status(404);
          req.send_header("Content-Type", "text/html");
          req.set_content_length(hello.size());
          req.send_body(hello);
        });
      }
  };
}
