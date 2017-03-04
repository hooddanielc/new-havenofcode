#pragma once

#include <string>
#include <stdexcept>
#include <hoc/json.h>
#include <hoc/route.h>

using namespace std;

namespace hoc {
  template<typename T>
  class register_route_t : public route_t<T> {
    public:
      register_route_t() : route_t<T>("/api/register") {}

      void post_end_invalid_json(const T &req) const {
        req.set_status(400);
        auto json = dj::json_t::empty_object;
        json["error"] = true;
        json["message"] = "invalid json";
        string out(json.to_string());
        req.set_content_length(out.size());
        req.send_body(out);
      }

      void post(const T &req, const url_match_result_t &) override {
        app_t &app = app_t::get();
        auto str = new string("");

        req.on_data([str](const std::string &data) mutable {
          str->append(data);
        });

        req.on_end([&, str]() mutable {
          req.send_header("Content-Type", "text/json");
          dj::json_t json;
          string email;
          string password;

          try {
            json = dj::json_t::from_string((*str).c_str());
            email = json["user"]["email"].to_string();
            password = json["user"]["password"].to_string();
            delete str;
          } catch (...) {
            delete str;
            return post_end_invalid_json(req);
          }

          if (password == "null" || email == "null") {
            return post_end_invalid_json(req);
          }

          cout << "succesfully got params" << endl;
          cout << email << endl;
          cout << password << endl;
        });
      }
  };
}
