#pragma once

#include <string>
#include <stdexcept>
#include <vector>
#include <hoc/json.h>
#include <hoc-db/db.h>

namespace hoc {
  template<typename T>
  class user_route_single_t : public route_t<T> {
    public:
      user_route_single_t() : route_t<T>("/api/users/:id") {}

      // this route allows only current user
      // to be selected
      void get(T &req, const url_match_result_t &match) {
        auto session = req.get_identity();

        if (session.has_identity() && match.params[0] == "me") {
          std::vector<db_param_t> params({
            session.id
          });

          auto user = session.db.exec(
            "SELECT U.id, U.email "
            "FROM hoc_session P "
            "INNER JOIN \"user\" U "
            "ON U.id = P.user "
            "WHERE P.id = $1 AND P.deleted = 'FALSE'",
            params
          );

          auto json = dj::json_t::empty_object;
          json["user"] = dj::json_t::empty_object;
          json["user"]["id"] = user[0][0].int_val();
          json["user"]["email"] = user[0][1].data();
          route_t<T>::send_json(req, json, 200);
        } else {
          return route_t<T>::fail_with_error(req, "login required");
        }
      }
  };

  template<typename T>
  class user_route_query_t : public route_t<T> {
    public:
      user_route_query_t() : route_t<T>("/api/users") {}

      void find_users_with_email(T &req, std::string &email) {
        auto json = dj::json_t::empty_object;
        dj::json_t::array_t users;

        db_t db;

        std::vector<db_param_t> params({
          email
        });

        auto res = db.exec(
          "SELECT id, email, active FROM \"user\" WHERE email = $1",
          params
        );

        for (int i = 0; i < res.rows(); ++i) {
          auto user = dj::json_t::empty_object;
          user["id"] = res[i][0].int_val();
          user["email"] = res[i][1].data();
          user["active"] = res[i][2].bool_val();
          users.emplace_back(std::move(user));
        }

        json["users"] = std::move(users);
        route_t<T>::send_json(req, json, 200);
      }

      void get(T &req, const url_match_result_t &) {
        req.on_end([&]()  {

          try {
            auto args = req.query();

            if (args.count("email")) {
              find_users_with_email(req, args["email"]);
            } else {
              route_t<T>::fail_with_error(req, "email query required");
            }
          } catch (const std::exception &e) {
            route_t<T>::fail_with_error(req, e.what());
          }
        });
      }
  };
} // hoc
