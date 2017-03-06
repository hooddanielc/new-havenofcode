#include <hoc-db/db.h>

using namespace hoc;
using namespace std;

namespace hoc {
  db_t::db_t() {
    conn = PQconnectdb(con_str().c_str());

    if (PQstatus(conn) != CONNECTION_OK) {
      PQfinish(conn);
      throw runtime_error(last_error());
    }
  }

  db_t::~db_t() {
    PQfinish(conn);
  }

  const char *db_t::status() {
    switch (PQstatus(conn)) {
      case CONNECTION_STARTED:
        return "CONNECTION_STARTED";
        break;
      case CONNECTION_MADE:
        return "CONNECTION_MADE";
        break;
      case CONNECTION_AWAITING_RESPONSE:
        return "CONNECTION_AWAITING_RESPONSE";
        break;
      case CONNECTION_AUTH_OK:
        return "CONNECTION_AUTH_OK";
        break;
      case CONNECTION_SSL_STARTUP:
        return "CONNECTION_SSL_STARTUP";
        break;
      case CONNECTION_SETENV:
        return "CONNECTION_SETENV";
        break;
      case CONNECTION_OK:
        return "CONNECTION OK";
        break;
      case CONNECTION_BAD:
        return "CONNECTION_BAD";
      case CONNECTION_NEEDED:
        return "CONNECTION_NEEDED";
        break;
      default:
        return "UNKNOWN";
    }
  }

  const char *db_t::last_error() {
    return PQerrorMessage(conn);
  }

  db_result_t db_t::exec(const char *query, const db_t::text_params_t &params) {
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
      string msg(PQerrorMessage(conn));
      throw runtime_error(msg);
    } else {
      return db_result_t(res);
    }
  }

  db_result_t db_t::exec(const char *query) {
    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) == PGRES_FATAL_ERROR) {
      string msg(PQerrorMessage(conn));
      throw runtime_error(msg);
    } else {
      return db_result_t(res);
    }
  }

  db_result_t db_t::exec(const char *query, const db_t::mixed_params_t &params) {
    const int size = params.size();
    int formats[size];
    int lengths[size];
    const char *values[size];

    for (int i = 0; i < size; ++i) {
      formats[i] = static_cast<int>(params[i].format());
      values[i] = params[i].val();
      lengths[i] = static_cast<int>(params[i].size());
    }

    PGresult *res = PQexecParams(
      conn,
      query,
      size,
      NULL,
      values,
      lengths,
      formats,
      1
    );

    if (PQresultStatus(res) == PGRES_FATAL_ERROR) {
      string msg(PQerrorMessage(conn));
      throw runtime_error(msg);
    } else {
      return db_result_t(res);
    }
  }

  bool db_t::connected() {
    return PQstatus(conn) == CONNECTION_OK;
  }

  const char *db_t::host() {
    return app_t::get().db_host;
  }

  const char *db_t::user() {
    return app_t::get().db_user;
  }

  const char *db_t::dbname() {
    return app_t::get().db_name;
  }

  const char *db_t::password() {
    return app_t::get().db_pass;
  }

  string db_t::con_str() {
    return string()
      .append("host=").append(host())
      .append(" dbname=").append(dbname())
      .append(" user=").append(user())
      .append(" password=").append(password())
      .append(" connect_timeout=10");
  }
}
