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
  env_t::get().db_name.set("hoc_test");
  env_t::get().db_user.set("admin_test");
  env_t::get().db_pass.set("123123");
  env_t::get().db_host.set("hoc-db");
  env_t::get().upload_tmp_path.set("/tmp");
  env_t::get().google_api_client_id.set("XXXXXXXX");
  env_t::get().google_api_client_secret.set("XXXXXXXX");
  env_t::get().aws_key.set("XXXXXXXX");
  env_t::get().aws_secret.set("XXXXXXXX");
  env_t::get().mock_s3_uploads.set("1");
  return dj::lick::main(argc, argv);
}

}  // hoc
