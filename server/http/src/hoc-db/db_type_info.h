#pragma once

#include <iostream>
#include <map>
#include <string>
#include <postgresql/libpq-fe.h>
#include <hoc-db/db.h>

namespace hoc {
  class db_type_info_t final {
    public:
      static db_type_info_t &get() {
        static db_type_info_t singleton;
        singleton.gen_types();
        return singleton;
      }

      const char *name(int oid);
      const char *category(int oid);

    private:
      std::map<const char *, const char *> names;
      std::map<const char *, const char *> categories;

      void gen_types();

      db_type_info_t(db_type_info_t &&) = delete;
      db_type_info_t(db_type_info_t &) = delete;
      db_type_info_t &operator=(db_type_info_t &&) = delete;
      db_type_info_t &operator=(db_type_info_t &) = delete;
      db_type_info_t() = default;
      ~db_type_info_t() = default;
  };
}
