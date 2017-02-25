#include <hoc-db/db_col.h>

using namespace std;

namespace hoc {
  char *db_col_t::data() {
    return PQgetvalue(res, row, col);
  }

  size_t db_col_t::size() {
    return PQgetlength(res, row, col);
  }

  bool db_col_t::binary() {
    return PQfformat(res, col);
  }

  char *db_col_t::name() {
    return PQfname(res, col);
  }

  int db_col_t::num() {
    return col;
  }

  bool db_col_t::empty() {
    return PQgetisnull(res, row, col);
  }

  int db_col_t::int_val() {
    return ntohl(*((uint32_t *) data()));
  }
}
