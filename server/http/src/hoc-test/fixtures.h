#pragma once
#include <hoc/db/connection.h>

namespace hoc {

inline void delete_test_accounts() {
  auto c = db::super_user_connection();
  pqxx::work w(*c);
  w.exec("delete from session_ip_log where id is not null");
  w.exec("delete from session where id is not null");
  w.exec("delete from registration where id is not null");
  w.exec("delete from account where id is not null");
  w.exec("drop user if exists" + w.quote_name("test_another@test.com"));
  w.exec("drop user if exists" + w.quote_name("test@test.com"));
  w.commit();
}

} // hoc
