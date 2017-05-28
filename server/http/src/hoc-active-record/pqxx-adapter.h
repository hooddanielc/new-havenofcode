#pragma once

#include <unordered_map>
#include <vector>
#include <tuple>
#include <memory>

namespace hoc {

template <typename obj_t, typename val_t>
class pqxx_adapter_t {
public:
  const primary_key_t<val_t> (obj_t::*p2m);

  pqxx_adapter_t(primary_key_t<val_t> (obj_t::*p2m_)) :
    p2m(p2m_),
    is_distinct(false),
    limit_rows(0),
    offset_rows(0) {}

  /*
   * distinct()
   * ============================================= */
  pqxx_adapter_t &distinct() {
    is_distinct = true;
    return *this;
  }

  /*
   * limit()
   * ============================================= */
  pqxx_adapter_t &limit(int limit_) {
    limit_rows = limit_;
    return *this;
  }

  /*
   * offset()
   * ============================================= */
  pqxx_adapter_t &offset(int offset_) {
    offset_rows = offset_;
    return *this;
  }

  /*
   * find(value)
   * ============================================= */
  pqxx_adapter_t &find(const val_t &val) {
    find_toks.push_back(str(val));
    return *this;
  }

  /*
   * find_by(key, value)
   * ============================================= */
  pqxx_adapter_t &find_by(const std::string &key, const std::string &val) {
    find_by_toks[key] = str(val);
    return *this;
  }

  // find_by on current obj
  template <typename col_val_t>
  pqxx_adapter_t &find_by(
    col_val_t (obj_t::*p2m),
    const decltype(col_val_t()) &val
  ) {
    auto name = obj_t::table.get_col_name(p2m);
    std::stringstream ss_name;
    ss_name << obj_t::table.name << "." << name;
    std::stringstream ss_val;
    ss_val << val;
    find_by_toks[ss_name.str()] = ss_val.str();
    return *this;
  }

  // find_by on current primary obj
  pqxx_adapter_t &find_by(
    primary_key_t<val_t> (obj_t::*p2m),
    const val_t &val
  ) {
    auto name = obj_t::table.get_col_name(p2m);
    std::stringstream ss_name;
    ss_name << obj_t::table.name << "." << name;
    std::stringstream ss_val;
    ss_val << val;
    find_by_toks[ss_name.str()] = ss_val.str();
    return *this;
  }

  // find_by on current foreign obj
  template <typename target_obj_t, typename col_val_t>
  pqxx_adapter_t &find_by(
    foreign_key_t<target_obj_t, col_val_t> (obj_t::*p2m),
    const decltype(col_val_t()) &val
  ) {
    auto name = obj_t::table.get_col_name(p2m);
    std::stringstream ss_name;
    ss_name << obj_t::table.name << "." << name;
    std::stringstream ss_val;
    ss_val << val;
    find_by_toks[ss_name.str()] = ss_val.str();
    return *this;
  }

  // find_by on other obj
  template <typename other_obj_t, typename other_val_t>
  pqxx_adapter_t &find_by(
    other_val_t (other_obj_t::*p2m),
    const decltype(other_val_t()) &val
  ) {
    auto name = other_obj_t::table.get_col_name(p2m);
    std::stringstream ss_name;
    ss_name << other_obj_t::table.name << "." << name;
    std::stringstream ss_val;
    ss_val << val;
    find_by_toks[ss_name.str()] = ss_val.str();
    return *this;
  }

  // find_by on other primary obj
  template <typename other_obj_t, typename other_val_t>
  pqxx_adapter_t &find_by(
    primary_key_t<other_val_t> (other_obj_t::*p2m),
    const decltype(other_val_t()) &val
  ) {
    auto name = other_obj_t::table.get_col_name(p2m);
    std::stringstream ss_name;
    ss_name << other_obj_t::table.name << "." << name;
    std::stringstream ss_val;
    ss_val << val;
    find_by_toks[ss_name.str()] = ss_val.str();
    return *this;
  }

  // find_by on other foreign obj
  template <typename other_obj_t, typename target_t, typename col_val_t>
  pqxx_adapter_t &find_by(
    foreign_key_t<target_t, col_val_t> (other_obj_t::*p2m),
    const decltype(col_val_t()) &val
  ) {
    auto name = other_obj_t::table.get_col_name(p2m);
    std::stringstream ss_name;
    ss_name << other_obj_t::table.name << "." << name;
    std::stringstream ss_val;
    ss_val << val;
    find_by_toks[ss_name.str()] = ss_val.str();
    return *this;
  }

  /*
   * where(condition)
   * ============================================= */
  pqxx_adapter_t &where(const std::string &condition) {
    where_toks.push_back(condition);
    return *this;
  }

  /*
   * order(key, direction = asc)
   * ============================================= */
  pqxx_adapter_t &order(const std::string &key, const std::string &direction = "") {
    order_toks[key] = direction;
    return *this;
  }

  template <typename other_obj_t, typename col_val_t>
  pqxx_adapter_t &order(
    col_val_t (other_obj_t::*p2m),
    const std::string &direction = ""
  ) {
    auto name = other_obj_t::table.get_col_name(p2m);
    std::stringstream ss_name;
    ss_name << other_obj_t::table.name << "." << name;
    order_toks[ss_name.str()] = direction;
    return *this;
  }

  template <typename other_obj_t, typename col_val_t>
  pqxx_adapter_t &order(
    primary_key_t<val_t> (other_obj_t::*p2m),
    const std::string &direction = ""
  ) {
    auto name = other_obj_t::table.get_col_name(p2m);
    std::stringstream ss_name;
    ss_name << other_obj_t::table.name << "." << name;
    order_toks[ss_name.str()] = direction;
    return *this;
  }

  template <typename other_obj_t, typename target_t, typename col_val_t>
  pqxx_adapter_t &order(
    foreign_key_t<target_t, col_val_t> (other_obj_t::*p2m),
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
  pqxx_adapter_t &joins(arg_t arg, more_t... more) {
    add_join(arg);
    return joins(more...);
  }

  pqxx_adapter_t &joins() {
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
    strm << "pqxx_adapter_t<" << obj_t::table.name << ">" << std::endl;
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
  std::string to_sql(
    std::shared_ptr<pqxx::connection> c = hoc::db::anonymous_connection()
  ) {
    std::stringstream ss;
    ss << "select ";

    if (is_distinct) {
      ss << "distinct ";
    }

    ss << obj_t::table.name << ".* from " << obj_t::table.name << " ";

    // eval joins
    for (const auto &join : join_toks) {
      ss << std::get<0>(join) << " " << std::get<1>(join) << " on "
         << std::get<2>(join) << " " << std::get<3>(join) << " ";
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
          ss << obj_t::table.name << "." << obj_t::table.get_primary_col_name() << " = ";
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
          ss << c->esc(it->first) << " = " << c->quote(it->second);
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
  pqxx_adapter_t &add_join(
    foreign_key_t<obj_t, val_t> (target_obj_t::*p2m)
  ) {
    std::stringstream ss_left;
    ss_left << obj_t::table.name << "." << obj_t::table.get_primary_col_name();
    auto target_col_name = target_obj_t::table.get_col_name(p2m);
    auto target_table_name = target_obj_t::table.name;
    std::stringstream ss_right;
    ss_right << target_table_name << "." << target_col_name;
    auto right_col = target_obj_t::table.get_col_name(p2m);
    join_toks.push_back(std::make_tuple(
      "inner join",
      target_obj_t::table.name,
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
  pqxx_adapter_t add_join(
    foreign_key_t<other_obj_t, other_val_t> (target_obj_t::*p2m)
  ) {
    std::stringstream ss_left;
    ss_left << other_obj_t::table.name << "." << other_obj_t::table.get_primary_col_name();
    auto target_col_name = target_obj_t::table.get_col_name(p2m);
    auto target_table_name = target_obj_t::table.name;
    std::stringstream ss_right;
    ss_right << target_table_name << "." << target_col_name;
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
          target_obj_t::table.name,
          left,
          ss_right.str()
        ));
        return *this;
      }
    }
    std::stringstream ss;
    ss << "must join primary key for " << target_obj_t::table.name << " first";
    throw ss.str();
  }

  template <typename col_val_t>
  std::string str(const col_val_t &val) {
    std::stringstream ss;
    ss << val;
    return ss.str();
  }

};  // pqxx_adapter_t<model_t>

template <typename obj_t, typename val_t>
pqxx_adapter_t<obj_t, val_t> make_adapter(
  primary_key_t<val_t> (obj_t::*p2m)
) {
  pqxx_adapter_t<obj_t, val_t> adapter(p2m);
  return adapter;
}

}   // hoc