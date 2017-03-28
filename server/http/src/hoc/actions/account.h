#pragma once

#include <sstream>
#include <pqxx/pqxx>
#include <hoc/env.h>
#include <hoc/db/connection.h>
#include <hoc/util.h>

namespace hoc {
namespace actions {

pqxx::result confirm_account(
  const std::string &email,
  const std::string &password
);

pqxx::result register_account(
  const std::string &email,
  const std::string &password
);

pqxx::result login(
  const std::string &email,
  const std::string &password,
  const std::string &ip = "none",
  const std::string &user_agent = "none"
);

pqxx::result restore_session(
  const std::string &uuid,
  const std::string &ip = "none",
  const std::string &user_agent = "none"
);

pqxx::result restore_session(
  std::shared_ptr<pqxx::connection> c,
  const std::string &uuid,
  const std::string &ip,
  const std::string &user_agent
);

} // actions
} // hoc