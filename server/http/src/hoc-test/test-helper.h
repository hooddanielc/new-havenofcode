#pragma once

#include <stdlib.h>
#include <hoc/env.h>
#include <lick/lick.h>
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
  w.exec("drop user if exists " + w.quote_name("test_another@test.com"));
  w.exec("drop user if exists " + w.quote_name("test@test.com"));
  w.commit();
}

inline int test_main(int argc, char *argv[]) {
  setenv("HOC_DB_NAME", "hoc_test", true);
  setenv("HOC_DB_USER", "admin_test", true);
  setenv("HOC_DB_PASSWORD", "123123", true);
  setenv("HOC_GOOGLE_API_CLIENT_ID", "XXXXXXXX", true);
  setenv("HOC_GOOGLE_API_CLIENT_SECRET", "XXXXXXXX", true);
  setenv("HOC_AWS_KEY", "XXXXXXXX", true);
  setenv("HOC_AWS_SECRET", "XXXXXXXX", true);
  setenv("HOC_MOCK_S3_UPLOADS", "1", true);
  return dj::lick::main(argc, argv);
}

}  // hoc
