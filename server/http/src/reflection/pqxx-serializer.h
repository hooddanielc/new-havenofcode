#pragma once

#include <stdexcept>
#include <unordered_map>
#include <string>
#include <sstream>
#include <reflection/serializer.h>
#include <hoc/db/connection.h>

template <typename obj_t>
class any_serializer_of_obj_t<obj_t, pqxx::tuple> {

public:

  ~any_serializer_of_obj_t() {
    postgres_oid = 0;
    column_order.clear();
  }

protected:

  static int postgres_oid;

  static std::unordered_map<int, std::string> column_order;

  static void read_table() {
    if (!postgres_oid) {
      auto c = hoc::db::super_user_connection();
      pqxx::work w(*c);
      std::stringstream ss_oid;
      ss_oid << "select '" << obj_t::name << "'::regclass::oid";
      auto oid = w.exec(ss_oid);
      postgres_oid = oid[0][0].as<int>();
      std::stringstream ss_column_order;
      ss_column_order << "select column_name, ordinal_position from information_schema.columns "
        "where table_schema = 'public' and table_name = '" << obj_t::name << "'";
      auto order = w.exec(ss_column_order);
      for (int i = 0; i < int(order.size()); ++i) {
        auto name = order[i][0].as<std::string>();
        auto num = order[i][1].as<int>();
        column_order[num - 1] = name;
      }
    }
  }
};

template <typename obj_t>
int any_serializer_of_obj_t<obj_t, pqxx::tuple>::postgres_oid = 0;

template <typename obj_t>
std::unordered_map<int, std::string> any_serializer_of_obj_t<obj_t, pqxx::tuple>::column_order;

// default pqxx serializer
template<typename val_t>
struct serialize_member_t<val_t, pqxx::tuple> {
  void operator()(const std::string &, const val_t &, pqxx::tuple &) {
    throw std::runtime_error("creating a tuple from model is pointless");
  }
};  // serialize_member_t<val_t, pqxx::tuple>

// default pqxx deserializer
template <typename val_t>
struct deserialize_member_t<val_t, pqxx::tuple> {
  void operator()(const std::string &, val_t &, const pqxx::tuple &) {
    throw std::runtime_error("won't do");
  }
};  // deserialize_member_t<val_t, hoc::json>

template <typename obj_t, typename val_t>
class pqxx_serializer_t: public member_serializer_t<obj_t, val_t, pqxx::tuple> {

public:

  using p2m_t = val_t (obj_t::*);

  pqxx_serializer_t(const std::string &name_, p2m_t ptr_) : member_serializer_t<obj_t, val_t, pqxx::tuple>(name_, ptr_), column_number(-1) {}

  using member_serializer_t<obj_t, val_t, pqxx::tuple>::member_serializer_t;

  virtual void write(const obj_t *, pqxx::tuple &) override {
    throw std::runtime_error("writing to pqxx::tuple is not allowed");
  }

  virtual void read(obj_t *obj, const pqxx::tuple &from) override {
    using any = any_serializer_of_obj_t<obj_t, pqxx::tuple>;
    read_column_number();

    if (any::postgres_oid) {
      for (int i = 0; i < int(from.size()); ++i) {
        if (int(from[i].table()) == any::postgres_oid && int(from[i].table_column()) == column_number) {
          obj->*(this->ptr) = from[i].as<val_t>();
          break;
        }
      }
    }
  }

private:

  int column_number;

  void read_column_number() {
    using any = any_serializer_of_obj_t<obj_t, pqxx::tuple>;
    any::read_table();

    if (column_number == -1) {
      for (auto it = any::column_order.begin(); it != any::column_order.end(); ++it) {
        if (it->second == this->name) {
          column_number = it->first;
        }
      }
    }
  }

};  // pqxx_serializer_t<obj_t, val_t>
