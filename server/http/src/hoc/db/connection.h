#pragma once

#include <memory>
#include <iostream>
#include <pqxx/pqxx>
#include <hoc/env.h>

namespace hoc {
namespace db {

static const std::string anonymous_con_str = 
  std::string("host=") + env_t::get().db_host +
  " dbname=" + env_t::get().db_name +
  " user=" + env_t::get().anonymous_user +
  " password=" + env_t::get().anonymous_pass;

static const std::string admin_con_str = 
  std::string("host=") + env_t::get().db_host +
  " dbname=" + env_t::get().db_name +
  " user=" + env_t::get().db_user +
  " password=" + env_t::get().db_pass;

inline std::shared_ptr<pqxx::connection> anonymous_connection() {
  return std::shared_ptr<pqxx::connection>(new pqxx::connection(anonymous_con_str));
}

inline std::shared_ptr<pqxx::connection> member_connection(const std::string &) {
  // todo
  return std::shared_ptr<pqxx::connection>(new pqxx::connection(anonymous_con_str));
}

inline std::shared_ptr<pqxx::connection> admin_connection() {
  return std::shared_ptr<pqxx::connection>(new pqxx::connection(admin_con_str));
}

}
}