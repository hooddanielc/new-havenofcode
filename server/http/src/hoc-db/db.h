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
#include <postgresql/libpq-fe.h>
#include <hoc/env.h>
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
      std::string con_str();
      const char *status();
      const char *last_error();
      bool connected();
      db_result_t exec(const char *query, text_params_t &params);
      db_result_t exec(const char *query, mixed_params_t &params);
      db_result_t exec(const char *query);
      std::string clean_literal(std::string str);
      std::string clean_identifier(std::string str);
    private:
      PGconn *conn;
  };
}
