#pragma once

#include <string>
#include <stdexcept>
#include <vector>
#include <hoc/json.h>
#include <hoc/route.h>
#include <hoc-db/db.h>
#include <hoc-crypto/rsa.h>
#include <hoc-crypto/sha-256.h>
#include <hoc-mail/mail.h>

namespace hoc {
  template<typename T>
  class register_route_t : public route_t<T> {
    public:
      register_route_t() : route_t<T>("/api/register") {}

      bool user_active(std::string &email, db_t &db) {
        std::vector<db_param_t> params({
          email,
          true
        });

        auto active_user = db.exec(
          "SELECT id FROM \"user\" WHERE email = $1 AND active = $2",
          params
        );

        return active_user.rows() == 1;
      }

      int get_user_id(std::string &email, db_t &db) {
        std::vector<db_param_t> email_params({ email });

        auto user = db.exec(
          "SELECT id FROM \"user\" WHERE email = $1",
          email_params
        );

        if (user.rows() == 0) {
          return db.exec(
            "INSERT INTO \"user\" (email) VALUES ($1) RETURNING id",
            email_params
          )[0][0].int_val();
        } else {
          // delete old registrations
          std::vector<db_param_t> user_id_params({ db_param_t(user[0][0].int_val()) });

          db.exec(
            "UPDATE registration SET deleted = 'TRUE' WHERE \"user\" = $1",
            user_id_params
          );
        }

        return user[0][0].int_val();
      }

      int refresh_registration(std::string &email, std::string &password, db_t &db) {
        // get or create user id
        int user = get_user_id(email, db);

        // get a random number
        auto random = crypto::use_entropy(64);
        std::ostringstream os;
        os << random;
        auto random_number = os.str();
        std::string salted_password = password + random_number;

        // hash the salted password
        auto hash = crypto::sha256(salted_password);

        std::vector<db_param_t> params({
          user,
          random_number,
          hash
        });

        // insert hash, and salt
        auto id = db.exec(
          "INSERT INTO registration (\"user\", \"secret\", \"password\")"
          "VALUES ($1, $2, $3)"
          "RETURNING user",
          params
        );

        // send an email to the user at the email address
        // with the encrypted message
        mail::send_registration_email(email, hash);
        return user;
      }

      void post(T &req, const url_match_result_t &) override {
        auto str = new std::string("");

        req.on_data([str](const std::string &data) mutable {
          str->append(data);
        });

        req.on_end([&, str]() mutable {
          req.send_header("Content-Type", "application/json");
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

          db_t db;
          db.exec("BEGIN TRANSACTION");

          try {
            if (user_active(email, db) == true) {
              return route_t<T>::fail_with_error(req, "user active");
            }

            int user_id = refresh_registration(email, password, db);
            db.exec("COMMIT");

            // done
            auto json = dj::json_t::empty_object;
            json["user"] = dj::json_t::empty_object;
            json["user"]["email"] = email;
            json["user"]["id"] = user_id;
            route_t<T>::send_json(req, json, 200);
          } catch (std::runtime_error e) {
            db.exec("ROLLBACK");
            throw e;
          }
        });
      }
  };
} // hoc
