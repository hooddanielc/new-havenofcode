#pragma once

#include <string>
#include <stdexcept>
#include <vector>
#include <hoc/json.h>
#include <hoc/route.h>
#include <hoc/actions/account.h>
#include <hoc-mail/mail.h>

namespace hoc {
  template<typename T>
  class register_route_t : public route_t<T> {
    public:
      register_route_t() : route_t<T>("/api/register") {}

      void post(T &req, const url_match_result_t &, std::shared_ptr<session_t<req_t>> &) override {
        auto str = new std::string("");

        req.on_data([str](const std::vector<uint8_t> &data) mutable {
          str->append(data.begin(), data.end());
        });

        req.on_end([&, str]() mutable {
          dj::json_t json;
          std::string email;
          std::string password;

          try {
            json = dj::json_t::from_string((*str).c_str());
            email = json["user"]["email"].as<std::string>();
            password = json["user"]["password"].as<std::string>();

            delete str;
          } catch (const std::exception &e) {
            return route_t<T>::fail_with_error(req, "invalid json");
          }

          if (password == "null" || email == "null") {
            return route_t<T>::fail_with_error(req, "username and password required");
          }

          try {
            auto r = actions::register_account(email, password);
            mail::send_registration_email(email, r[0][1].as<std::string>());
            auto result = dj::json_t::empty_object;
            result["user"] = dj::json_t::empty_object;
            result["user"]["email"] = email;
            result["user"]["id"] = r[0][0].as<std::string>();
            route_t<T>::send_json(req, result, 200);
          } catch (const std::exception &e) {
            route_t<T>::fail_with_error(req, e.what());
          }
        });
      }
  };
} // hoc
