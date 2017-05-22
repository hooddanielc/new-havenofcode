#pragma once

#include <sstream>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <pqxx/pqxx>
#include <hoc/env.h>
#include <hoc/util.h>

namespace hoc {
namespace db {

inline static const std::string get_anonymous_con_str() {
  static const std::string anonymous_con_str = 
    std::string("host=") + env_t::get().db_host +
    " dbname=" + env_t::get().db_name +
    " user=" + env_t::get().anonymous_user +
    " password=" + env_t::get().anonymous_pass;
  return anonymous_con_str;
}

inline static const std::string get_admin_con_str() {
  static const std::string admin_con_str = 
    std::string("host=") + env_t::get().db_host +
    " dbname=" + env_t::get().db_name +
    " user=" + env_t::get().db_user +
    " password=" + env_t::get().db_pass;
  return admin_con_str;
}

inline std::shared_ptr<pqxx::connection> anonymous_connection() {
  return std::shared_ptr<pqxx::connection>(new pqxx::connection(get_anonymous_con_str()));
}

inline std::shared_ptr<pqxx::connection> member_connection(
  const std::string &email,
  const std::string &password
) {
  auto c = anonymous_connection();
  pqxx::work w(*c);
  std::stringstream ss;

  ss << "select salt from account where email = " << w.quote(email);
  auto r = w.exec(ss);

  if (r.size() == 0) {
    throw std::runtime_error("incorrect user or password");
  }

  std::string salted_password = password + r[0][0].as<std::string>();
  ss.str(std::string());
  ss.clear();
  ss << "select md5(" << w.quote(salted_password) << ")";
  auto r_for_hash = w.exec(ss);

  const std::string con_str = 
    std::string("host=") + env_t::get().db_host +
    " dbname=" + env_t::get().db_name +
    " user=" + email +
    " password=" + r_for_hash[0][0].as<std::string>();

  std::shared_ptr<pqxx::connection> c_for_user(new pqxx::connection(con_str));
  pqxx::work w_for_user(*c_for_user);

  ss.str(std::string());
  ss.clear();
  std::string new_salt(random_characters(32));
  ss << "update account set salt = " << w.quote(new_salt) << " "
     << "where email = " << w.quote(email);
  w_for_user.exec(ss);

  ss.str(std::string());
  ss.clear();
  ss << "select md5(" << w.quote(password + new_salt) << ")";
  auto r_for_new_hash = w_for_user.exec(ss);

  ss.str(std::string());
  ss.clear();
  ss << "alter user " << w.quote_name(email) << " "
     << "with password " << w.quote(r_for_new_hash[0][0].as<std::string>());
  w_for_user.exec(ss);

  w_for_user.commit();
  return c_for_user;
}

inline std::shared_ptr<pqxx::connection> super_user_connection() {
  return std::shared_ptr<pqxx::connection>(new pqxx::connection(get_admin_con_str()));
}

inline std::shared_ptr<pqxx::connection> session_connection(const std::string &session_id) {
  auto c = super_user_connection();
  pqxx::work w(*c);
  std::stringstream ss;
  ss << "select account.email from session, account where "
     << "session.created_by = account.id and "
     << "session.id = " << w.quote(session_id) << " and "
     << "session.deleted = 'FALSE'";

  auto r = w.exec(ss);

  if (r.size() == 0) {
    throw std::runtime_error("invalid session");
  }

  auto email = r[0][0].as<std::string>();
  ss.str(std::string());
  ss.clear();
  ss << "set role " << w.quote_name(email);
  w.exec(ss);

  ss.str(std::string());
  ss.clear();
  ss << "set session authorization " << w.quote_name(email);
  w.exec(ss);

  w.commit();
  return c;
}

} // db
} // hoc