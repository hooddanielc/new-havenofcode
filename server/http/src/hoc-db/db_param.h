#pragma once

#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <functional>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace hoc {
  class db_param_t;

  enum db_param_type_t {
    db_param_int,
    db_param_str
  };

  class db_param_cb_t final {
    friend db_param_t;
    public:
      using bool_cb_t = std::function<bool(db_param_t &)>;
      using size_cb_t = std::function<size_t(db_param_t &)>;
      using char_cb_t = std::function<const char *(db_param_t &)>;

      static db_param_cb_t &get() {
        static db_param_cb_t singleton;
        singleton.gen_cbs();
        return singleton;
      }
    private:
      std::map<db_param_type_t, bool_cb_t> bool_cbs;
      std::map<db_param_type_t, size_cb_t> size_cbs;
      std::map<db_param_type_t, char_cb_t> char_cbs;
      void gen_cbs();

      db_param_cb_t(){};
      db_param_cb_t(db_param_cb_t &&) = delete;
      db_param_cb_t(const db_param_cb_t &) = delete;
      ~db_param_cb_t() = default;
  };

  class db_param_t final {
    friend class db_param_cb_t;
    public:
      db_param_t(int num) : cache(nullptr), type(db_param_int), num_val(num) {}
      db_param_t(const char *str) : cache(nullptr), type(db_param_str), str_val(str) {}
      db_param_t(const std::string &str) : cache(nullptr), type(db_param_str), str_val(str) {}

      ~db_param_t() {
        free(cache);
      }

      size_t size() const;
      const char *val() const;
      bool format() const;
      db_param_type_t get_type() const;
    private:
      char *cache;
      db_param_type_t type;
      int num_val;
      std::string str_val;
  };
}
