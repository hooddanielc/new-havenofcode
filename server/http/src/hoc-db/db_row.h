#pragma once

#include <iostream>
#include <stdlib.h>
#include <postgresql/libpq-fe.h>
#include <hoc-db/db_col.h>

namespace hoc {
  class db_result_t;

  class db_row_t final {
    friend db_result_t;
    public:
      db_col_t operator[](int col) const {
        return db_col_t(res, row, col);
      }

    private:
      int row;
      PGresult *res;
      db_row_t(PGresult *res, int row) : row(row), res(res) {};
      db_row_t &operator=(db_row_t &) = delete;
  };
}
