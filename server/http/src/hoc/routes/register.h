#pragma once

#include <string>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <hoc/json.h>
#include <hoc/route.h>
#include <hoc-crypto/crypto.h>

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

      void post_end_invalid_user_json(const T &req) const {
        req.set_status(400);
        auto json = dj::json_t::empty_object;
        json["error"] = true;
        json["message"] = "user active";
        string out(json.to_string());
        req.set_content_length(out.size());
        req.send_body(out);
      }

      void post_end_critical_error(const T &req) const {
        req.set_status(500);
        auto json = dj::json_t::empty_object;
        json["error"] = true;
        json["message"] = "database error";
        string out(json.to_string());
        req.set_content_length(out.size());
        req.send_body(out);
      }

      bool user_active(const string &email, db_t &db) {
        auto active_user = db.exec(
          "SELECT id FROM \"user\" WHERE email = $1 AND active = $2",
          vector<db_param_t>({
            email,
            true
          })
        );

        return active_user.rows() == 1;
      }

      db_result_t refresh_registration(const string &email, db_t &db) {
        auto user = db.exec(
          "SELECT id FROM \"user\" WHERE email = $1",
          vector<db_param_t>({ email })
        );

        if (user.rows() == 0) {
          user = db.exec(
            "INSERT INTO \"user\" (email) VALUES ($1) RETURNING id",
            vector<db_param_t>({ email })
          );
        } else {
          // delete old registrations
          db.exec(
            "UPDATE registration SET deleted = 'TRUE' WHERE \"user\" = $1",
            vector<db_param_t>({
              user[0][0].int_val()
            })
          );
        }

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
        db.exec(
          "INSERT INTO registration (\"user\", \"rsaPubD\", \"rsaPubN\", \"secret\")"
          "VALUES ($1, $2, $3, $4)"
          "RETURNING id",
          vector<db_param_t>({
            user[0][0].int_val(),
            string(pri.get_k1().get_str()),
            string(pri.get_k2().get_str()),
            string(secret_message.get_str())
          })
        );

        // send an email to the user at the email address
        // with the encrypted message
        

        return user;
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
            email = json["user"]["email"].as<string>();
            password = json["user"]["password"].as<string>();
            delete str;
          } catch (...) {
            delete str;
            return post_end_invalid_json(req);
          }

          if (password == "null" || email == "null") {
            return post_end_invalid_json(req);
          }

          try {
            db_t db;
            db.exec("BEGIN TRANSACTION");

            if (user_active(email, db) == true) {
              return post_end_invalid_user_json(req);
            }

            refresh_registration(email, db);
            db.exec("COMMIT");
          } catch (runtime_error e) {
            cout << e.what() << endl;
            return post_end_critical_error(req);
          }
        });
      }
  };
}