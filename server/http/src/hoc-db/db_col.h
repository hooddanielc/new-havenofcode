#pragma once

#include <stdlib.h>
#include <netinet/in.h>
#include <libpq-fe.h>

namespace hoc {
  class db_row_t;

  class db_col_t final {
    friend class db_row_t;
    public:
      char *data();
      size_t size();
      bool binary();
      char *name();
      int num();
      bool empty();
      int int_val();
      bool bool_val();

    private:
      PGresult *res;
      int row;
      int col;
      db_col_t(PGresult *res, int row, int col) : res(res), row(row), col(col) {};
  };
}
