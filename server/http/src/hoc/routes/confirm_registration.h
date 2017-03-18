#pragma once

#include <hoc/route.h>
#include <hoc-crypto/sha-256.h>

namespace hoc {
  template<typename T>
  class confirm_registration_route_t : public route_t<T> {
    public:
      confirm_registration_route_t() : route_t<T>("/api/confirm-registration") {}

      void post(T &req, const url_match_result_t &) override {
        auto str = new std::string();

        req.on_data([str](const std::string &data) {
          str->append(data);
        });

        req.on_end([&, str]() {
          dj::json_t json;
          std::string email;
          std::string password;
          std::string confirm_password;
          std::string salt;
          int user_id;

          try {
            json = dj::json_t::from_string(str->c_str());

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

          db_t db;

          // verify the salt
          try {
            db.exec("BEGIN TRANSACTION");

            // get the user id and the salt
            std::vector<db_param_t> get_user_params({
              email,
              password
            });

            auto get_user_result = db.exec(
              "SELECT C.id, P.secret, P.id "
              "FROM registration P "
              "INNER JOIN \"user\" C "
              "ON C.id = P.user "
              "WHERE C.email = $1 AND P.password = $2 AND P.deleted = 'FALSE'",
              get_user_params
            );

            if (get_user_result.rows() == 0) {
              return route_t<T>::fail_with_error(req, "registration unknown");
            }

            user_id = get_user_result[0][0].int_val();
            salt = get_user_result[0][1].data();
            int registration_id = get_user_result[0][2].int_val();

            // verify the hashed salt and password match
            if (crypto::sha256(confirm_password + salt) != password) {
              return route_t<T>::fail_with_error(req, "password mismatch");
            }

            std::vector<db_param_t> delete_registration_params({
              db_param_t(registration_id)
            });

            db.exec(
              "UPDATE registration SET deleted = 'TRUE' WHERE id = $1",
              delete_registration_params
            );

            std::string query("CREATE USER ");

            query
              .append(db.clean_identifier(email))
              .append(" WITH PASSWORD ")
              .append(db.clean_literal(confirm_password));

            db.exec(query.c_str());

            std::string grant_query = "GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO ";
            grant_query.append(db.clean_identifier(email));

            db.exec(grant_query.c_str());

            std::vector<db_param_t> set_user_active_params({
              db_param_t(user_id)
            });

            db.exec("COMMIT");
            db.exec("END TRANSACTION");
            auto success_json = dj::json_t::empty_object;
            success_json["user"] = dj::json_t::empty_object;
            success_json["user"]["email"] = email;
            success_json["user"]["active"] = true;
            route_t<T>::send_json(req, success_json, 200);
          } catch (std::runtime_error e) {
            db.exec("ROLLBACK");
            std::cout << e.what() << std::endl;
            auto json = dj::json_t::empty_object;
            json["error"] = true;
            json["message"] = e.what();
            route_t<T>::send_json(req, json, 500);
          }
        });
      }
  };
}
