#pragma once
#include <hoc/db/connection.h>
#include <hoc/actions/account.h>

namespace hoc {

inline void delete_all_user_data() {
  auto c = db::super_user_connection();
  pqxx::work w(*c);
  w.exec("delete from session_ip_log where id is not null");
  w.exec("delete from session where id is not null");
  w.exec("delete from registration where id is not null");
  w.exec("delete from file_part_promise where id is not null");
  w.exec("delete from file_part where id is not null");
  w.exec("delete from file where id is not null");
  w.exec("delete from account where id is not null");
  w.exec("drop user if exists" + w.quote_name("test_another@test.com"));
  w.exec("drop user if exists" + w.quote_name("test@test.com"));
  w.commit();
}

inline void create_user_steve() {
  actions::register_account("test_user_steve@test.com", "password");
  actions::confirm_account("test_user_steve@test.com", "password");
  auto c = db::member_connection("test_user_steve@test.com", "password");
}

} // hoc
