#pragma once

#include <vector>
#include <libpq-fe.h>
#include <hoc-db/db_row.h>
#include <hoc/json.h>
#include <cstring>

namespace hoc {
  class db_t;

  class db_result_t final {
    friend class db_t;
    public:
      const db_row_t operator[](int row) const {
        return db_row_t(res, row);
      }

      int cols();
      std::vector<const char *> field_names();
      int rows();
      ~db_result_t();
    private:
      PGresult *res;
      db_result_t(PGresult *res) : res(res) {};
  };
}
