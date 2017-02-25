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

namespace hoc {
  class db_result_t;
  class db_t;
  class db_row_t;

  class db_t final {
    using params_t = std::vector<const char *>;
    public:
      db_t();
      ~db_t();
      const char *host();
      const char *user();
      const char *dbname();
      const char *password();
      const char *con_str();
      bool connected();
      db_result_t exec(const char *query, const params_t &params);
      db_result_t exec(const char *query);
    private:
      PGconn *conn;
  };

  class db_row_t final {
    friend db_result_t;
    public:
      const char * operator[](int col) const {
        return PQgetvalue(res, row, col);
      }

    private:
      int row;
      PGresult *res;
      db_row_t(PGresult *res, int row) : row(row), res(res) {};
  };

  class db_result_t final {
    friend class db_t;
    public:
      const db_row_t operator[](int row) const {
        return db_row_t(res, row);
      }

      int fields();
      int rows();
      ~db_result_t();
    private:
      PGresult *res;
      db_result_t(PGresult *res) : res(res) {};
  };
}
