#include <iostream>
#include <cstdlib>
#include <string>
#include <libpq-fe.h>
#include <lick/lick.h>
#include <hoc-db/db.h>

using namespace hoc;
using namespace std;

FIXTURE(environment_variables_found) {
  EXPECT_EQ(string(getenv("HOC_DB_NAME")), "hoc_dev");
  EXPECT_EQ(string(getenv("HOC_DB_HOST")), "hoc-db");
  EXPECT_EQ(string(getenv("HOC_DB_USER")), "admin_dev");
  EXPECT_EQ(string(getenv("HOC_DB_PASSWORD")), "123123");
}

FIXTURE(db_t_uses_envs) {
  db_t db;
  EXPECT_EQ(string(db.dbname()), "hoc_dev");
  EXPECT_EQ(string(db.host()), "hoc-db");
  EXPECT_EQ(string(db.user()), "admin_dev");
  EXPECT_EQ(string(db.password()), "123123");
}

FIXTURE(db_t_con_str) {
  db_t db;
  EXPECT_EQ(string(db.con_str()), "host = hoc-db dbname = hoc_dev user = admin_dev password = 123123");
}

FIXTURE(db_t_is_connected) {
  db_t db;
  EXPECT_EQ(db.connected(), true);
}

FIXTURE(db_does_query) {
  db_t db;
  EXPECT_EQ(db.connected(), true);
  db.exec("BEGIN");
  db.exec("DECLARE myportal CURSOR FOR select * from pg_database");
  db_result_t res = db.exec("FETCH ALL in myportal");
  EXPECT_GT(res.fields(), 0);
  EXPECT_GT(res.rows(), 0);
  db.exec("END");
}

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}
