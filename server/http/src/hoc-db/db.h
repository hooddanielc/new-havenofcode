#pragma once

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <libpq-fe.h>
#include <hoc-db/db_result.h>
#include <hoc-db/db_param.h>

namespace hoc {
  class db_t final {
    using text_params_t = std::vector<const char *>;
    using mixed_params_t = std::vector<db_param_t>;

    public:
      db_t();
      ~db_t();
      const char *host();
      const char *user();
      const char *dbname();
      const char *password();
      const char *con_str();
      bool connected();
      db_result_t exec(const char *query, const text_params_t &params);
      db_result_t exec(const char *query, const mixed_params_t &params);
      db_result_t exec(const char *query);
    private:
      PGconn *conn;
  };
}
