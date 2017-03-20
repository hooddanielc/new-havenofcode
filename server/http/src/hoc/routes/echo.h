#pragma once

#include <hoc/route.h>

namespace hoc {
  template<typename T>
  class echo_route_t : public route_t<T> {
    public:
      echo_route_t() : route_t<T>("/api/echo") {}

      void serve(T &req) {
        auto str = new std::string();
        str->append(req.request_line());

        for (auto it = req.request_headers.begin(); it != req.request_headers.end(); ++it) {
          for (auto const &header_value : it->second) {
            str->append("\n").append(it->first).append(" ").append(header_value);
          }
        }

        str->append("\n\n");

        req.on_data([str](const std::string &data) {
          str->append(data);
        });

        req.on_end([&, str]() {
          str->append("\n\n");
          str->append("ip: ").append(req.ip());
          req.get_identity();
          req.set_status(200);
          req.send_header("Content-Type", "text/html");
          req.set_content_length(str->size());
          req.send_body(*str);
          delete str;
        });
      }

      void get(T &req, const url_match_result_t &) override {
        serve(req);
      }

      void post(T &req, const url_match_result_t &) override {
        serve(req);
      }

      void put(T &req, const url_match_result_t &) override {
        serve(req);
      }

      void del(T &req, const url_match_result_t &) override {
        serve(req);
      }
  };
}
