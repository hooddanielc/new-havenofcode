#include <iostream>
#include <cstdlib>
#include <string>
#include <libpq-fe.h>
#include <vector>
#include <lick/lick.h>
#include <hoc-db/db.h>
#include <hoc-db/db_param.h>
#include <hoc-db/db_type_info.h>

using namespace hoc;
using namespace std;

void refresh() {
  db_t db;
  db.exec("BEGIN");
  db.exec("DELETE FROM registration WHERE id > 0");
  db.exec("DELETE FROM article WHERE id > 0");
  db.exec("DELETE FROM \"user\" WHERE id > 0");
  db.exec("END");
}

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
  EXPECT_EQ(
    string(db.con_str()),
    "host=hoc-db port=5432 dbname=hoc_dev user=admin_dev password=123123 connect_timeout=1000"
  );
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
  EXPECT_GT(res.cols(), 0);
  EXPECT_GT(res.rows(), 0);
  db.exec("END");
}

FIXTURE(db_gets_type_info) {
  db_type_info_t::get();
}

FIXTURE(db_does_query_params) {
  EXPECT_EQ(string(getenv("HOC_DB_NAME")), "hoc_dev");
  EXPECT_EQ(string(getenv("HOC_DB_HOST")), "hoc-db");
  EXPECT_EQ(string(getenv("HOC_DB_USER")), "admin_dev");
  EXPECT_EQ(string(getenv("HOC_DB_PASSWORD")), "123123");
  refresh();
  db_t db;
  vector<const char *> params;
  params.push_back("email@email.com");

  db.exec("BEGIN");
  db.exec("INSERT INTO \"user\" (email) VALUES ($1)", params);
  db.exec("END");

  db.exec("BEGIN");
  db_result_t res = db.exec("SELECT email FROM \"user\" WHERE email = $1", params);
  db.exec("DELETE FROM \"user\" WHERE email = $1", params);
  db.exec("END");

  EXPECT_EQ(res.cols(), 1);
  EXPECT_EQ(res.rows(), 1);
  EXPECT_EQ(int(res.field_names().size()), 1);
  EXPECT_EQ(string(res.field_names()[0]), "email");
  EXPECT_EQ(string(res[0][0].data()), "email@email.com");
}

FIXTURE(db_nice_error_msg) {
  refresh();
  db_t db;
  db.exec("BEGIN");
  db.exec("DELETE FROM \"user\" WHERE id > 0");
  db.exec("DELETE FROM article WHERE id > 0");
  db.exec("END");
  vector<const char *> params;
  params.push_back("email@email.com");

  try {
    db.exec("LOL should not work");
  } catch (runtime_error e) {
    EXPECT_EQ(
      e.what(),
      string(
        "ERROR:  syntax error at or near \"LOL\"\n"
        "LINE 1: LOL should not work\n"
        "        ^\n"
      )
    );
  }
}

FIXTURE(db_does_mixed_params) {
  refresh();
  db_t db;
  db.exec("BEGIN");
  db.exec("DELETE FROM \"user\" WHERE id > 0");
  db.exec("DELETE FROM article WHERE id > 0");
  db.exec("END");

  const char *email = "user@user.com";
  const char *title = "asdf";
  const char *description = "1234";
  const char *md = "entire article";

  vector<const char *> user_params({ email });

  vector<db_param_t> article_insert_params({
    string(title),
    string(description),
    string(md)
  });

  // insert user
  db.exec("BEGIN");
  db.exec("INSERT INTO \"user\" (email) VALUES ($1)", user_params);
  db.exec("END");

  // select user using email
  db.exec("BEGIN");
  auto user_res = db.exec("SELECT id, email FROM \"user\" WHERE email = $1", user_params);
  db.exec("END");

  // insert article
  db.exec("BEGIN");
  article_insert_params.push_back(user_res[0][0].int_val());
  db.exec("INSERT INTO article (title, description, md, \"user\") VALUES ($1, $2, $3, $4)", article_insert_params);
  db.exec("END");

  // select user using id
  db.exec("BEGIN");
  auto user_res_two = db.exec("SELECT * FROM \"user\" WHERE id = $1", vector<db_param_t>({
    user_res[0][0].int_val()
  }));
  db.exec("END");

  // select article using title
  auto article_res = db.exec("SELECT id FROM article WHERE title = $1", vector<db_param_t>({
    string(title)
  }));

  EXPECT_EQ(user_res[0][0].int_val(), user_res_two[0][0].int_val());
}

FIXTURE(db_does_joins) {
  refresh();
  db_t db;

  // insert user
  db.exec("BEGIN");
  auto user = db.exec(
    "INSERT INTO \"user\" (email) VALUES ($1) RETURNING id",
    vector<db_param_t>({ "email@email.com" })
  );
  db.exec("END");

  // insert article
  db.exec("BEGIN");
  auto article = db.exec(
    "INSERT INTO article (title, description, md, \"user\") VALUES ($1, $2, $3, $4)"
    "RETURNING id",
    vector<db_param_t>({ "asdf", "description", "md", user[0][0].int_val() })
  );
  db.exec("END");

  db.exec("BEGIN");
  auto joined = db.exec(
    "SELECT C.email "
    "FROM article P "
    "INNER JOIN \"user\" C "
    "ON C.id = P.user "
    "WHERE C.id = $1",
    vector<db_param_t>({ user[0][0].int_val() })
  );
  db.exec("END");

  auto field_names = joined.field_names();

  EXPECT_EQ(string("email"), field_names[0]);
  EXPECT_EQ(string("email@email.com"), joined[0][0].data());
}

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}
