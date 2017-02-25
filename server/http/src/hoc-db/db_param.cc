#include <hoc-db/db_param.h>

using namespace std;

namespace hoc {
  void db_param_cb_t::gen_cbs() {
    // db_param_int
    bool_cbs[db_param_int] = [](db_param_t &) {
      return true;
    };

    size_cbs[db_param_int] = [](db_param_t &) {
      return sizeof(int);
    };

    char_cbs[db_param_int] = [](db_param_t &param) {
      uint32_t binaryIntVal = htonl((uint32_t) param.num_val);
      param.cache = (char *) new uint32_t(binaryIntVal);
      return param.cache;
    };

    // db_param_str
    bool_cbs[db_param_str] = [](db_param_t &) {
      return false;
    };

    size_cbs[db_param_str] = [](db_param_t &param) {
      return param.str_val.size();
    };

    char_cbs[db_param_str] = [](db_param_t &param) {
      return param.str_val.c_str();
    };
  }

  size_t db_param_t::size() const {
    return db_param_cb_t::get().size_cbs[type](*const_cast<db_param_t*>(this));
  }

  const char *db_param_t::val() const {
    return db_param_cb_t::get().char_cbs[type](*const_cast<db_param_t*>(this));
  }

  bool db_param_t::format() const {
    return db_param_cb_t::get().bool_cbs[type](*const_cast<db_param_t*>(this));
  }

  db_param_type_t db_param_t::get_type() const {
    return type;
  }
}
