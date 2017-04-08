#pragma once

#include <sstream>
#include <hoc/route.h>
#include <hoc/actions/account.h>

namespace hoc {
  template<typename T>
  class confirm_registration_route_t : public route_t<T> {
    public:
      confirm_registration_route_t() : route_t<T>("/api/confirm-registration") {}

      void post(T &req, const url_match_result_t &, std::shared_ptr<session_t<req_t>> &session) override {
        auto str = new std::string();

        req.on_data([str](const std::string &data) {
          str->append(data);
        });

        req.on_end([&, str, session]() {
          std::string email;
          std::string password;
          std::string confirm_password;

          try {
            auto json = dj::json_t::from_string(str->c_str());

            if (!json.contains("user")) {
              return route_t<T>::fail_with_error(req, "user model required");
            } else if (!json["user"].contains("email")) {
              return route_t<T>::fail_with_error(req, "user email required");
            } else if (!json["user"].contains("password")) {
              return route_t<T>::fail_with_error(req, "user password required");
            } else if (!json["user"].contains("confirmPassword")) {
              return route_t<T>::fail_with_error(req, "user plain password required");
            }

            email = json["user"]["email"].as<std::string>();
            password = json["user"]["password"].as<std::string>();
            confirm_password = json["user"]["confirmPassword"].as<std::string>();

            delete str;
          } catch (std::exception) {
            delete str;
            return route_t<T>::fail_with_error(req, "json malformed");
          }

          try {
            pqxx::work w(*session->db);
            std::stringstream ss;
            ss << "select email from registration where "
               << "email = " << w.quote(email) << " and "
               << "password = " << w.quote(password) << " and "
               << "verified = 'FALSE'";
            auto r_for_count = w.exec(ss);
            if (r_for_count.size() == 0) {
              return route_t<T>::fail_with_error(req, "Registration not found", 404);
            }
            auto res = actions::confirm_account(email, confirm_password);
            auto json = dj::json_t::empty_object;
            json["user"] = dj::json_t::empty_object;
            json["user"]["email"] = res[0][1].as<std::string>();
            json["user"]["id"] = res[0][0].as<std::string>();
            route_t<T>::send_json(req, json, 200);
          } catch (std::runtime_error e) {
            route_t<T>::fail_with_error(req, e.what());
          }
        });
      }
  };
}
