#pragma once

#include <stdexcept>
#include <unordered_map>
#include <string>
#include <sstream>
#include <reflex/reflection.h>
#include <reflex/member.h>
#include <hoc/db/connection.h>

namespace reflex {

template <typename obj_t>
class pqxx_table_t {

public:

  static void read_table() {
    if (postgres_oid == -1) {
      auto c = hoc::db::super_user_connection();
      pqxx::work w(*c);
      std::stringstream ss_oid;
      ss_oid << "select '" << obj_t::reflection.name << "'::regclass::oid";
      auto oid = w.exec(ss_oid);
      postgres_oid = oid[0][0].as<int>();
      std::stringstream ss_column_order;
      ss_column_order << "select column_name, ordinal_position from information_schema.columns "
        "where table_schema = 'public' and table_name = '" << obj_t::reflection.name << "'";
      auto order = w.exec(ss_column_order);
      for (int i = 0; i < int(order.size()); ++i) {
        auto name = order[i][0].as<std::string>();
        auto num = order[i][1].as<int>();
        column_order[name] = num - 1;
      }
    }
  }

  static std::unordered_map<std::string, int> get_column_order() {
    return column_order;
  }

  static int get_postgres_oid() {
    return postgres_oid;
  }

private:

  pqxx_table_t() = default;

  ~pqxx_table_t() = default;

  static int postgres_oid;

  static std::unordered_map<std::string, int> column_order;

};  // pqxx_table_t

template <typename obj_t>
int pqxx_table_t<obj_t>::postgres_oid = -1;

template <typename obj_t>
std::unordered_map<std::string, int> pqxx_table_t<obj_t>::column_order;

template <typename obj_t, typename val_t>
struct member_helper_t<obj_t, val_t, pqxx::tuple> {

  static constexpr bool is_specialized { true };

  static void write(const std::string &, const val_t &, const pqxx::tuple &) {
    throw std::runtime_error("can not write to pqxx::tuple");
  }

  static void read(const std::string &name, val_t &val, const pqxx::tuple &from) {
    using def_t = pqxx_table_t<obj_t>;
    read_column_number(name);

    for (int i = 0; i < int(from.size()); ++i) {
      if (int(from[i].table()) == def_t::get_postgres_oid() && int(from[i].table_column()) == column_number) {
        if (!from[i].is_null()) {
          val = from[i].as<val_t>();
        }
        break;
      }
    }
  }

  static int column_number;

  static void read_column_number(const std::string &name) {
    using def_t = pqxx_table_t<obj_t>;
    def_t::read_table();

    if (column_number == -1) {
      auto order = def_t::get_column_order();

      if (!order.count(name)) {
        std::stringstream ss;
        ss << "postgres table `" << obj_t::reflection.name << "` column `" << name << "` does not exist";
        throw std::runtime_error(ss.str());
      }

      column_number = order[name];
    }
  }

};  // member_helper_t<obj_t, val_t, pqxx::tuple>

template <typename obj_t, typename val_t>
int member_helper_t<obj_t, val_t, pqxx::tuple>::column_number = -1;

}   // reflex
