#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <stdexcept>

#include <hoc/util.h>
#include <hoc-db/db.h>

namespace hoc {
  template<typename T>
  class session_t {
    private:
      bool authenticated;
    public:
      db_t db;
      const std::string id;
      session_t(
        const std::string &_id,
        bool _authenticated
      ) : authenticated(_authenticated),
          id(_id) {};

      bool has_identity() {
        return authenticated;
      }

      static session_t<T> refresh_session(T &req, db_t &db) {
        std::string ip = req.ip();
        string user_agent = "none";

        if (req.request_headers.count("User-Agent") > 0) {
          user_agent = req.request_headers["User-Agent"][0];
        }

        auto create_session_result = db.exec(
          "INSERT INTO hoc_session (id) "
          "VALUES (uuid_generate_v4()) "
          "RETURNING id"
        );

        std::string session_id(create_session_result[0][0].data());

        std::vector<db_param_t> create_log_params({
          session_id,
          ip,
          user_agent
        });

        db.exec(
          "INSERT INTO session_ip_log "
          "(id, session, ip, \"userAgent\") "
          "VALUES (uuid_generate_v4(), $1, $2, $3)",
          create_log_params
        );

        std::string session_cookie("session=");
        session_cookie.append(session_id);
        session_cookie.append("; Expires=Wed, 21 Oct 2030 07:28:00 GMT");
        session_cookie.append("; Path=/");

        req.send_header("Set-Cookie", session_cookie);
        db.exec("COMMIT");

        session_t<T> result(session_id, false);
        return result;
      }

      static session_t<T> make(T &req) {
        db_t db;
        db.exec("BEGIN TRANSACTION");
        auto cookies = req.cookies();
        std::string user_agent = "none";

        if (req.request_headers.count("User-Agent") > 0) {
          user_agent = req.request_headers["User-Agent"][0];
        }

        if (cookies.count("session")) {
          if (cookies["session"].size() > 1) {
            throw std::logic_error("multiple session keys");
          }

          std::string session_id = cookies["session"][0];

          std::vector<db_param_t> get_session_params({
            session_id
          });

          auto session_result = db.exec(
            "SELECT C.id, P.id, P.ip, P.\"userAgent\", U.email "
            "FROM session_ip_log P "
            "INNER JOIN \"hoc_session\" C "
            "ON C.id = P.session "
            "INNER JOIN \"user\" U "
            "ON U.id = C.user "
            "WHERE C.id = $1",
            get_session_params
          );

          if (session_result.rows() == 0) {
            // try to find existing ip logs
            // for session
            auto get_unauthenticated_sessions = db.exec(
              "SELECT P.id, P.ip, P.\"userAgent\" "
              "FROM session_ip_log P "
              "INNER JOIN \"hoc_session\" C "
              "ON C.id = P.session "
              "WHERE C.id = $1",
              get_session_params
            );

            if (get_unauthenticated_sessions.rows() == 0) {
              // session doesn't exist, create new one
              return refresh_session(req, db);
            } else {
              session_t<T> result(session_id, false);

              // get ips associated with session
              auto get_ip_logs_result = result.db.exec(
                "SELECT ip, \"userAgent\", id::varchar FROM session_ip_log WHERE session = $1",
                get_session_params
              );

              std::string ip = req.ip();
              bool found = false;
              bool user_agent_wrong = false;

              for (int i = 0; i < get_ip_logs_result.rows(); ++i) {
                // find the ip
                std::string logged_ip = get_ip_logs_result[i][0].data();

                if (logged_ip == ip) {
                  string agent = get_ip_logs_result[i][1].data();

                  // found the ip, update session log
                  string update_visit_id = get_ip_logs_result[i][2].data();

                  std::vector<db_param_t> update_ip_params({
                    update_visit_id
                  });

                  result.db.exec("BEGIN");
                  result.db.exec(
                    "UPDATE session_ip_log SET \"updatedAt\" = 'NOW()' WHERE id = $1",
                    update_ip_params
                  );
                  result.db.exec("END");

                  if (agent != user_agent) {
                    user_agent_wrong = true;
                  }

                  found = true;

                  // set last update timestamp
                  break;
                }
              }

              // insert a new sessino ip
              if (user_agent_wrong) {
                throw runtime_error("user agent");
              } else if (!found) {
                std::vector<db_param_t> asdf({
                  session_id,
                  ip,
                  user_agent
                });

                result.db.exec("BEGIN");
                result.db.exec(
                  "INSERT INTO session_ip_log "
                  "(id, session, ip, \"userAgent\") "
                  "VALUES (uuid_generate_v4(), $1, $2, $3)",
                  asdf
                );
                result.db.exec("END");
              }

              return result;
            }
          } else {
            session_t<T> result(session_id, true);
            std::string query("SET ROLE ");
            query.append(result.db.clean_identifier(session_result[0][4].data()));
            std::string query_session("SET SESSION AUTHORIZATION ");
            query_session.append(result.db.clean_identifier(session_result[0][4].data()));
            result.db.exec("BEGIN");
            result.db.exec(query.c_str());
            result.db.exec(query_session.c_str());
            result.db.exec("END");
            return result;
          }
        } else {
          // session cookie missing, create a new one
          return refresh_session(req, db);
        }
      }
  };
}
