#pragma once

#include <unordered_map>
#include <vector>
#include <tuple>
#include <memory>
#include <hoc-active-record/model.h>

namespace hoc {

template <typename obj_t, typename val_t>
class sql_adapter_t {
public:
  using array_t = std::vector<std::shared_ptr<obj_t>>;
  const primary_key_t<val_t> (obj_t::*p2m);

  sql_adapter_t(primary_key_t<val_t> (obj_t::*p2m_)) :
    p2m(p2m_),
    is_distinct(false),
    limit_rows(0),
    offset_rows(0) {}

  /*
   * distinct()
   * ============================================= */
  sql_adapter_t &distinct() {
    is_distinct = true;
    return *this;
  }

  /*
   * limit()
   * ============================================= */
  sql_adapter_t &limit(int limit_) {
    limit_rows = limit_;
    return *this;
  }

  /*
   * offset()
   * ============================================= */
  sql_adapter_t &offset(int offset_) {
    offset_rows = offset_;
    return *this;
  }

  /*
   * find(value)
   * ============================================= */
  sql_adapter_t &find(const val_t &val) {
    find_toks.push_back(str(val));
    return *this;
  }

  /*
   * find_by(key, value)
   * ============================================= */
  sql_adapter_t &find_by(const std::string &key, const std::string &val) {
    auto c = store_t::get()->get_connection();
    find_by_toks[key] = c->quote(str(val));
    return *this;
  }

  template <typename other_obj_t, typename other_val_t>
  sql_adapter_t &find_by(
    other_val_t (other_obj_t::*p2m),
    const std::string &val
  ) {
    auto c = store_t::get()->get_connection();
    auto name = other_obj_t::table.get_col_name(p2m);
    std::stringstream ss_name;
    ss_name << c->quote_name(other_obj_t::table.name) << "." << c->quote_name(name);
    find_by_toks[ss_name.str()] = c->quote(val);
    return *this;
  }

  /*
   * where(condition)
   * ============================================= */
  sql_adapter_t &where(const std::string &condition) {
    where_toks.push_back(condition);
    return *this;
  }

  /*
   * order(key, direction = asc)
   * ============================================= */
  sql_adapter_t &order(const std::string &key, const std::string &direction = "") {
    order_toks[key] = direction;
    return *this;
  }

  template <typename other_obj_t, typename col_val_t>
  sql_adapter_t &order(
    col_val_t (other_obj_t::*p2m),
    const std::string &direction = ""
  ) {
    auto name = other_obj_t::table.get_col_name(p2m);
    std::stringstream ss_name;
    ss_name << other_obj_t::table.name << "." << name;
    order_toks[ss_name.str()] = direction;
    return *this;
  }

  /*
   * joins
   * ============================== */
  template <typename arg_t, typename... more_t>
  sql_adapter_t &joins(arg_t arg, more_t... more) {
    add_join(arg);
    return joins(more...);
  }

  sql_adapter_t &joins() {
    return *this;
  }

  /*
   * print debug output
   * ======================= */
  template <typename list_type_t>
  void print_list(const std::string name, std::vector<list_type_t> &list, std::ostream &strm) {
    strm << "  " << name << std::endl;
    for (const auto &item : list) {
      strm << "    " << item << std::endl;
    }
    strm << std::endl;
  }

  void print_map(const std::string &name, std::unordered_map<std::string, std::string> &list, std::ostream &strm) {
    strm << "  " << name << std::endl;
    for (const auto &item : list) {
      strm << "    " << item.first << " : " << item.second << std::endl;
    }
    strm << std::endl;
  }

  void print_joins(std::ostream &strm) {
    strm << "  " << std::endl;
    for (const auto &item : join_toks) {
      strm << "    " << std::get<0>(item) << std::endl;
      strm << "      " << std::get<1>(item) << std::endl;
      strm << "      " << std::get<2>(item) << std::endl;
      strm << "      " << std::get<3>(item) << std::endl;
    }
    strm << std::endl;
  }

  void write(std::ostream &strm) {
    strm << "sql_adapter_t<" << obj_t::table.name << ">" << std::endl;
    print_list("find_toks", find_toks, strm);
    print_list("where_toks", where_toks, strm);
    print_map("find_by_toks", find_by_toks, strm);
    print_map("order_toks", order_toks, strm);
    print_joins(strm);
  }

  // todo create_with
  // todo group
  // todo having
  // todo includes
  // todo references
  // todo like

  /*
   * to_sql();
   * ======================= */
  std::string to_sql(std::shared_ptr<pqxx::connection> c = store_t::get()->get_connection()) {
    pqxx::work w(*c);
    std::stringstream ss;
    ss << "select ";

    if (is_distinct) {
      ss << "distinct ";
    }

    ss << "* from " << c->quote_name(obj_t::table.name) << " ";

    // eval joins
    for (const auto &join : join_toks) {
      ss << std::get<0>(join) << " " << std::get<1>(join) << " on "
         << std::get<2>(join) << " = " << std::get<3>(join) << " ";
    }

    bool needs_where = find_toks.size() ||
      find_by_toks.size() ||
      where_toks.size() ||
      join_toks.size();

    if (needs_where) {
      ss << "where";

      if (find_toks.size()) {
        ss << " (";
        for (auto it = find_toks.begin(); it != find_toks.end(); ++it) {
          ss << c->quote_name(obj_t::table.name) << "." << c->quote_name(obj_t::table.get_primary_col_name()) << " = ";
          ss << c->quote(*it);
          if (std::next(it) != find_toks.end()) {
            ss << " or ";
          }
        }
        ss << ")";
      }

      if (where_toks.size()) {
        if (find_toks.size()) {
          ss << " or (";
        } else {
          ss << " (";
        }
        for (auto it = where_toks.begin(); it != where_toks.end(); ++it) {
          ss << *it;
          if (std::next(it) != where_toks.end()) {
            ss << " or ";
          }
        }
        ss << ")";
      }

      if (find_by_toks.size()) {
        if (find_toks.size() || where_toks.size()) {
          ss << " and ";
        } else {
          ss << " ";
        }

        for (auto it = find_by_toks.begin(); it != find_by_toks.end(); ++it) {
          ss << it->first << " = " << it->second;
          if (std::next(it) != find_by_toks.end()) {
            ss << " and ";
          }
        }
      }
    }

    if (limit_rows) {
      ss << " limit " << limit_rows;
    }

    if (offset_rows) {
      ss << " offset " << offset_rows;
    }

    return ss.str();
  }

  /*
   * array_t operator=()
   *
   * queries the database and uses factories to return an
   * array of records
   */
  operator array_t () {
    // todo
    array_t result;
    return result;
  }

private:
  bool is_distinct;
  int limit_rows;
  int offset_rows;
  std::vector<val_t> find_toks;
  std::vector<std::string> where_toks;
  std::unordered_map<std::string, std::string> find_by_toks;
  std::unordered_map<std::string, std::string> order_toks;
  std::vector<std::tuple<
    std::string,  // join type
    std::string,  // joined table
    std::string,  // column name
    std::string   // column name
  >> join_toks;

  // add join for current table
  template <typename target_obj_t>
  sql_adapter_t &add_join(
    foreign_key_t<obj_t, val_t> (target_obj_t::*p2m)
  ) {
    auto c = store_t::get()->get_connection();
    std::stringstream ss_left;
    ss_left << c->quote_name(obj_t::table.name) << "." << c->quote_name(obj_t::table.get_primary_col_name());
    auto target_col_name = target_obj_t::table.get_col_name(p2m);
    auto target_table_name = target_obj_t::table.name;
    std::stringstream ss_right;
    ss_right << c->quote_name(target_table_name) << "." << c->quote_name(target_col_name);
    auto right_col = target_obj_t::table.get_col_name(p2m);
    join_toks.push_back(std::make_tuple(
      "inner join",
      c->quote_name(target_obj_t::table.name),
      ss_left.str(),
      ss_right.str()
    ));
    return *this;
  }

  template<typename out_t>
  void split(const std::string &s, char delim, out_t &result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
      result.push_back(item);
    }
  }

  // allow nested joins
  template <typename other_obj_t, typename other_val_t, typename target_obj_t>
  sql_adapter_t add_join(
    foreign_key_t<other_obj_t, other_val_t> (target_obj_t::*p2m)
  ) {
    auto c = store_t::get()->get_connection();
    std::stringstream ss_left;
    ss_left << c->quote_name(other_obj_t::table.name) << "." << c->quote_name(other_obj_t::table.get_primary_col_name());
    auto target_col_name = target_obj_t::table.get_col_name(p2m);
    auto target_table_name = target_obj_t::table.name;
    std::stringstream ss_right;
    ss_right << c->quote_name(target_table_name) << "." << c->quote_name(target_col_name);
    auto right_col = target_obj_t::table.get_col_name(p2m);

    // check if referenced key is included in
    // join, otherwise throw error explaining
    // that nested join queries do not work
    // unless all tables link with each other
    for (const auto &item : join_toks) {
      std::vector<std::string> res;
      auto left = ss_left.str();
      split(left, '.', res);
      if (res.size() > 0 && res[0] == std::get<1>(item)) {
        join_toks.push_back(std::make_tuple(
          "inner join",
          c->quote_name(target_obj_t::table.name),
          left,
          ss_right.str()
        ));
        return *this;
      }
    }
    std::stringstream ss;
    ss << "must join primary key for " << c->quote_name(target_obj_t::table.name) << " first";
    throw ss.str();
  }

  template <typename col_val_t>
  std::string str(const col_val_t &val) {
    std::stringstream ss;
    ss << val;
    return ss.str();
  }

};  // sql_adapter_t<model_t>

template <typename obj_t, typename val_t>
sql_adapter_t<obj_t, val_t> make_sql_adapter(
  primary_key_t<val_t> (obj_t::*p2m)
) {
  sql_adapter_t<obj_t, val_t> adapter(p2m);
  return adapter;
}

}   // hoc
