#include <hoc-db/db_result.h>

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
}
