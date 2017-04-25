#pragma once

#include <hoc/route.h>
#include <hoc/json.h>
#include <hoc/session.h>

namespace hoc {
  template<typename T>
  class login_route_t : public route_t<T> {
    public:
      login_route_t() : route_t<T>("/api/login") {}

      void post(T &req, const url_match_result_t &, std::shared_ptr<session_t<req_t>> &session) override {
        std::string *str = new std::string();

        req.on_data([str](const std::vector<uint8_t> &data) {
          str->append(data.begin(), data.end());
        });

        req.on_end([&, str, session]() {
          // is the user is already authenticated?
          if (session->authenticated()) {
            return route_t<T>::fail_with_error(req, "user already authenticated");
          }

          std::string email;
          std::string password;

          try {
            auto json = dj::json_t::from_string(str->c_str());

            if (!json.contains("email")) {
              return route_t<T>::fail_with_error(req, "email required");
            } else if (!json.contains("password")) {
              return route_t<T>::fail_with_error(req, "password required");
            }

            email = json["email"].as<std::string>();
            password = json["password"].as<std::string>();
            delete str;
          } catch (const std::exception &e) {
            delete str;
            return route_t<T>::fail_with_error(req, e.what());
          }

          // try to log into database with email and password
          try {
            session->login(req, email, password);
            auto result = dj::json_t::empty_object;
            result["user"] = dj::json_t::empty_object;
            result["user"]["id"] = session->user_id();
            result["user"]["session"] = session->id();
            route_t<T>::send_json(req, result, 200);
          } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            return route_t<T>::fail_with_error(req, "unauthorized", 403);
          }
        });
      }
  };
}
