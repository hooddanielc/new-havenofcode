#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include <map>
#include <stdexcept>
#include <hoc/db/connection.h>

namespace hoc {

template <typename req_t>
class session_t {
  private:
    std::string session_id;
    std::string account_id;
    bool is_authenticated;

    void create_new_session(req_t &req) {
      db = db::anonymous_connection();
      pqxx::work w(*db);
      std::stringstream ss;
      auto ip = req.ip();
      auto user_agent = req.user_agent();
      ss << "insert into session (ip, user_agent) values ("
         << w.quote(ip) << ","
         << w.quote(user_agent)
         << ") returning id";
      auto r_for_session = w.exec(ss);
      session_id = r_for_session[0][0].as<std::string>();
      ss.str(std::string());
      ss.clear();

      ss << "insert into session_ip_log (ip, user_agent, session) values ("
         << w.quote(ip) << ","
         << w.quote(user_agent) << ","
         << w.quote(r_for_session[0][0].as<std::string>())
         << ")";

      w.exec(ss);
      w.commit();
      is_authenticated = false;
      send_session_id(req, r_for_session[0][0].as<std::string>());
    }

    void send_session_id(req_t &req, const std::string &id) {
      std::string session_header = "session=";
      session_header.append(id).append("; Expires=Wed, 21 Oct 2030 07:28:00 GMT; Path=/");
      req.send_header("Set-Cookie", session_header);
    }

    void restore_session(
      req_t &req,
      std::map<std::string, std::vector<std::string>> cookies
    ) {
      auto temp_c = db::super_user_connection();
      pqxx::work w(*temp_c);
      auto session_cookie = cookies["session"][0];
      std::stringstream ss;
      ss << "select created_by, id,"
         << "user_agent from session where "
         << "id = " << w.quote(session_cookie) << " and "
         << "deleted = 'FALSE'";
      auto r_current_session = w.exec(ss);

      if (r_current_session.size() == 0) {
        create_new_session(req);
      } else {
        auto user_agent = req.user_agent();

        if (user_agent != r_current_session[0][2].as<std::string>()) {
          ss.str(std::string());
          ss.clear();
          ss << "udpate session set deleted = 'TRUE' "
             << "where id = " << r_current_session[0][1].as<std::string>();
          w.exec(ss);
          w.commit();
          throw std::runtime_error("user agent changed");
        }

        auto ip = req.ip();
        if (r_current_session[0][0].is_null()) {
          w.exec("set role anonymous");
          w.exec("set session authorization anonymous");
        } else {
          ss.str(std::string());
          ss.clear();
          ss << "select email from account where id = " << w.quote(r_current_session[0][0].as<std::string>());
          auto r_email = w.exec(ss);
          ss.str(std::string());
          ss.clear();
          ss << "set role " << w.quote_name(r_email[0][0].as<std::string>());
          w.exec(ss);
          ss.str(std::string());
          ss.clear();
          ss << "set session authorization "
             << w.quote_name(r_email[0][0].as<std::string>());
          w.exec(ss);
          ss.str(std::string());
          ss.clear();
          ss << "insert into session_ip_log (ip, user_agent, session, created_by) values ("
             << w.quote(ip) << ","
             << w.quote(user_agent) << ","
             << w.quote(r_current_session[0][1].as<std::string>()) << ","
             << "current_account_id()"
             << ")";
        }

        w.exec(ss);
        ss.str(std::string());
        ss.clear();
        ss << "update session set updated_at = 'NOW()', "
           << "ip = " << w.quote(ip) << " "
           << "where id = " << w.quote(r_current_session[0][1].as<std::string>());
        w.exec(ss);
        w.commit();
        session_id = r_current_session[0][1].as<std::string>();

        if (!r_current_session[0][0].is_null()) {
          is_authenticated = true;
          account_id = r_current_session[0][0].as<std::string>();
        }

        db = temp_c;
      }
    }

  public:
    std::shared_ptr<pqxx::connection> db;
    session_t() = delete;

    // create an anonymous session
    session_t(req_t &req) : is_authenticated(false) {
      auto cookies = req.cookies();

      if (cookies.count("session") && cookies["session"].size() == 1) {
        restore_session(req, cookies);
      } else {
        create_new_session(req);
      }
    }

    // create session authenticated session
    // using username and password.
    // should throw if connection fails.
    session_t(
      req_t &req,
      const std::string &email,
      const std::string &password
    ) {
      login(req, email, password);
    }

    void login(
      req_t &req,
      const std::string &email,
      const std::string &password
    ) {
      db = db::member_connection(email, password);
      is_authenticated = true;
      pqxx::work w(*db);
      auto ip = req.ip();
      auto user_agent = req.user_agent();
      std::stringstream ss;
      ss << "insert into session (ip, user_agent, created_by) VALUES ("
         << w.quote(ip) << ","
         << w.quote(user_agent) << ","
         << "current_account_id()"
         << ") returning id, created_by";
      auto r_for_session = w.exec(ss);
      session_id = r_for_session[0][0].as<std::string>();
      account_id = r_for_session[0][1].as<std::string>();
      ss.str(std::string());
      ss.clear();
      ss << "insert into session_ip_log (ip, user_agent, session, created_by) values ("
         << w.quote(ip) << ","
         << w.quote(user_agent) << ","
         << w.quote(r_for_session[0][0].as<std::string>()) << ","
         << "current_account_id()"
         << ")";
      w.exec(ss);
      w.commit();
      send_session_id(req, r_for_session[0][0].as<std::string>());
    }

    std::string id() {
      return session_id;
    }

    bool authenticated() {
      return is_authenticated;
    }

    std::string user_id() {
      return account_id;
    }

    void logout(req_t &req) {
      pqxx::work w(*db);
      std::stringstream ss;
      ss << "update session set deleted = 'TRUE' where "
         << "id = " << w.quote(session_id);
      w.exec(ss);
      w.commit();
      create_new_session(req);
    }

    static std::shared_ptr<session_t> make(req_t &req) {
      return std::shared_ptr<session_t>(new session_t(req));
    }
};

} // hoc
