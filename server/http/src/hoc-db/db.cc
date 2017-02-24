#include <hoc-db/db.h>

using namespace hoc;
using namespace std;

#include <iostream>

namespace hoc {
  db_result_t::~db_result_t() {
    PQclear(res);
  }

  int db_result_t::fields() {
    return PQnfields(res);
  }

  int db_result_t::rows() {
    return PQntuples(res);
  }

  db_t::db_t() {
    conn = PQconnectdb(con_str());
  }

  db_t::~db_t() {
    PQfinish(conn);
  }

  db_result_t db_t::exec(const char *query, const db_t::params_t &params) {
    PGresult *res = PQexecParams(
      conn,
      query,
      params.size(),
      NULL,
      &params[0],
      NULL,
      NULL,
      1
    );

    if (PQresultStatus(res) == PGRES_FATAL_ERROR) {
      throw runtime_error(string("postgres error `") + PQerrorMessage(conn) + "`");
    } else {
      return db_result_t(res);
    }
  }

  db_result_t db_t::exec(const char *query) {
    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) == PGRES_FATAL_ERROR) {
      string msg;

      msg
        .append("postgres command failed `")
        .append(query)
        .append("` with `")
        .append(PQerrorMessage(conn))
        .append("`");

      throw runtime_error(msg);
    } else {
      return db_result_t(res);
    }
  }

  bool db_t::connected() {
    return PQstatus(conn) == CONNECTION_OK;
  }

  const char *db_t::host() {
    return getenv("HOC_DB_HOST");
  }

  const char *db_t::user() {
    return getenv("HOC_DB_USER");
  }

  const char *db_t::dbname() {
    return getenv("HOC_DB_NAME");
  }

  const char *db_t::password() {
    return getenv("HOC_DB_PASSWORD");
  }

  const char *db_t::con_str() {
    return string()
      .append("host = ").append(host())
      .append(" dbname = ").append(dbname())
      .append(" user = ").append(user())
      .append(" password = ").append(password()).c_str();
  }
}
