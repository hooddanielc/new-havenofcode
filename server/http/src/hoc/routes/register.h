#pragma once

#include <string>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <hoc/json.h>
#include <hoc/route.h>
#include <hoc-db/db.h>
#include <hoc-crypto/crypto.h>

using namespace std;

namespace hoc {
  template<typename T>
  class register_route_t : public route_t<T> {
    public:
      register_route_t() : route_t<T>("/api/register") {}

      bool user_active(string &email, db_t &db) {
        vector<db_param_t> params({
          email,
          true
        });

        auto active_user = db.exec(
          "SELECT id FROM \"user\" WHERE email = $1 AND active = $2",
          params
        );

        return active_user.rows() == 1;
      }

      int get_user_id(string &email, db_t &db) {
        vector<db_param_t> email_params({ email });

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
          vector<db_param_t> user_id_params({ db_param_t(user[0][0].int_val()) });

          db.exec(
            "UPDATE registration SET deleted = 'TRUE' WHERE \"user\" = $1",
            user_id_params
          );
        }

        return user[0][0].int_val();
      }

      int refresh_registration(string &email, db_t &db) {
        int user = get_user_id(email, db);

        // insert a new registration with private key
        crypto::rsa_crypto_t keys;
        auto pri = keys.get_private_key();
        auto pub = keys.get_public_key();
        auto secret_message = crypto::huge_random_number(64);
        mpz_class encrypted_message;

        // encrypt with public key, then throw the public
        // key away for ever
        mpz_powm(
          encrypted_message.get_mpz_t(),
          secret_message.get_mpz_t(),
          pub.get_k1().get_mpz_t(),
          pub.get_k2().get_mpz_t()
        );

        // insert private key into database
        // along with the secret message
        string a(pri.get_k1().get_str());
        string b(string(pri.get_k2().get_str()));
        string c(string(secret_message.get_str()));
        auto params = vector<db_param_t>({
          db_param_t(user),
          db_param_t(a),
          db_param_t(b),
          db_param_t(c)
        });

        auto id = db.exec(
          "INSERT INTO registration (\"user\", \"rsaPubD\", \"rsaPubN\", \"secret\")"
          "VALUES ($1, $2, $3, $4)"
          "RETURNING user",
          params
        );

        // send an email to the user at the email address
        // with the encrypted message
        app_t::get().send_registration_email(email, encrypted_message.get_str());
        return user;
      }

      void post(T &req, const url_match_result_t &) override {
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
            email = json["user"]["email"].as<string>();
            password = json["user"]["password"].as<string>();
            delete str;
          } catch (const exception &e) {
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

            int user_id = refresh_registration(email, db);
            db.exec("COMMIT");

            // done
            auto json = dj::json_t::empty_object;
            json["user"] = dj::json_t::empty_object;
            json["user"]["email"] = email;
            json["user"]["id"] = user_id;
            route_t<T>::send_json(req, json, 200);
          } catch (runtime_error e) {
            db.exec("ROLLBACK");
            throw e;
          }
        });
      }
  };
}
