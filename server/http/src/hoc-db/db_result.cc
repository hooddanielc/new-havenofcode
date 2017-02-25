#include <hoc-db/db_result.h>

using namespace std;

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

  vector<const char *> db_result_t::field_names() {
    vector<const char *> names;

    for (int i = 0; i < fields(); ++i) {
      names.push_back(PQfname(res, i));
    }

    return names;
  }
}
