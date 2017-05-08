#include <lick/lick.h>
#include <vector>
#include <pqxx/pqxx>

using namespace std;

class property_t {
public:
  const std::string name;
  const std::string type_name;
  const std::string belongs_to_key;

  property_t(const std::string &_name) :
    name(_name),
    type_name("unknown") {}
  property_t(
    const std::string &_name,
    const std::string &_type_name
  ) :
    name(_name),
    type_name(_type_name) {}
  property_t(
    const std::string &_name,
    const std::string &_type_name,
    const std::string &_belongs_to_key
  ) :
    name(_name),
    type_name(_type_name),
    belongs_to_key(_belongs_to_key) {}

  property_t &operator=(property_t other) {
    return other;
  }
};

class query_builder_t {
private:
  std::map<std::string, std::vector<property_t>> tables;
  std::vector<
    std::tuple<std::string, std::string, std::string, std::string>
  > joins;
  std::string where_stmt;
  std::string order_stmt;

public:
  template <typename schema_t>
  query_builder_t &select() {
    if (!tables.count(schema_t::name)) {
      tables[schema_t::name] = schema_t::attrs;
    }

    return *this;
  }

  template <typename schema_t, typename schema_on_t>
  query_builder_t &join(const std::string &from, const std::string &to) {
    if (!tables.count(schema_t::name)) {
      tables[schema_t::name] = schema_t::attrs;
    }

    if (!tables.count(schema_on_t::name)) {
      tables[schema_on_t::name] = schema_on_t::attrs;
    }

    joins.emplace_back(std::make_tuple(schema_t::name, from, schema_on_t::name, to));
    return *this;
  }

  query_builder_t &order(const std::string &order_by) {
    order_stmt = order_by;
    return *this;
  }

  query_builder_t &where(const std::string &where_statement) {
    where_stmt = where_statement;
    return *this;
  }

  std::string to_string() {
    if (!tables.size()) {
      return "";
    }

    std::stringstream ss;
    ss << "select ";
    for (auto it = tables.begin(); it != tables.end(); ++it) {
      for (auto iter = it->second.begin(); iter != it->second.end(); ++iter) {
        if (it != tables.begin() || iter != it->second.begin()) {
          ss << ", ";
        }

        ss << it->first << "." << (*iter).name;
      }
    }

    std::vector<std::string> taboo;
    taboo.push_back(tables.begin()->first);
    ss << " from " << tables.begin()->first << " ";

    for (auto it = joins.begin(); it != joins.end(); ++it) {
      std::string joiner;
      std::string first(std::get<0>(*it));
      std::string first_key(std::get<1>(*it));
      std::string second(std::get<2>(*it));
      std::string second_key(std::get<3>(*it));

      if (std::find(taboo.begin(), taboo.end(), first) == taboo.end()) {
        joiner = first;
      } else {
        joiner = second;
      }

      taboo.push_back(joiner);

      ss << "inner join " << joiner << " on " << first << "." << first_key << " "
         << "= " << second << "." << second_key << " ";
    }

    if (!where_stmt.empty()) {
      ss << "where " << where_stmt << " ";
    }

    if (!order_stmt.empty()) {
      ss << "order by " << order_stmt;
    }

    return ss.str();
  }
};

template <typename schema_t>
class model_t {
public:
  static void print_schema() {
    std::cout << "table: " << schema_t::name << std::endl;
    for (const property_t &prop : schema_t::attrs) {
      if (prop.belongs_to_key.empty()) {
        std::cout << "  " << prop.name << " : " << prop.type_name << std::endl;
      } else {
        std::cout << "  belongs to " << prop.type_name << " on " << prop.name << std::endl;
      }
    }
  }

  model_t() {
    std::cout << "Im a model" << std::endl;
  }
};

template <typename T> struct type_as_string;
template <> struct type_as_string<int> {
  static constexpr const char *value() {
    return "int";
  }
};
template <> struct type_as_string<std::string> {
  static constexpr const char *value() {
    return "std::string";
  }
};
template <> struct type_as_string<char*> {
  static constexpr const char *value() {
    return "char*";
  }
};
template <> struct type_as_string<const char*> {
  static constexpr const char *value() {
    return "const char*";
  }
};
template <> struct type_as_string<bool> {
  static constexpr const char *value() {
    return "bool";
  }
};
template <> struct type_as_string<long> {
  static constexpr const char *value() {
    return "long";
  }
};
template <> struct type_as_string<long long> {
  static constexpr const char *value() {
    return "long long";
  }
};
template <> struct type_as_string<unsigned long> {
  static constexpr const char *value() {
    return "unsigned long";
  }
};
template <> struct type_as_string<unsigned long long> {
  static constexpr const char *value() {
    return "unsigned long long";
  }
};

template <typename type_t>
class attribute_t : public property_t {
public:
  attribute_t(const std::string &_name) : property_t::property_t(
    _name,
    type_as_string<type_t>::value()
  ) {}
};

template <typename schema_t>
class belongs_to_t : public property_t {
public:
  belongs_to_t(
    const std::string &_name,
    const std::string &_foreign_key
  ) : property_t::property_t(
    _name,
    std::string("schema::") + schema_t::name,
    _foreign_key
  ) {}
};

class files_schema_t {
public:
  static const char name[];
  static const std::vector<property_t> attrs;
};

const char files_schema_t::name[] = "file";
const std::vector<property_t> files_schema_t::attrs = {
  attribute_t<std::string>("id"),
  attribute_t<std::string>("created_by"),
  attribute_t<std::string>("created_at"),
  attribute_t<std::string>("updated_at"),
  attribute_t<std::string>("type"),
  attribute_t<std::string>("name"),
  attribute_t<std::string>("upload_id"),
  attribute_t<std::string>("aws_key"),
  attribute_t<std::string>("aws_region"),
  attribute_t<std::string>("aws_bucket"),
  attribute_t<unsigned long>("bytes"),
  attribute_t<std::string>("status")
};

class file_parts_schema_t {
public:
  static const char name[];
  static const std::vector<property_t> attrs;
};

const char file_parts_schema_t::name[] = "file_part";
const std::vector<property_t> file_parts_schema_t::attrs = {
  attribute_t<std::string>("id"),
  attribute_t<std::string>("created_at"),
  attribute_t<std::string>("updated_at"),
  attribute_t<bool>("deleted"),
  attribute_t<unsigned long>("bytes"),
  attribute_t<std::string>("aws_etag"),
  attribute_t<std::string>("aws_part_number"),
  attribute_t<bool>("pending"),
  attribute_t<std::string>("created_by"),
  attribute_t<std::string>("file")
};

FIXTURE(creates) {
  model_t<files_schema_t> model;
}

FIXTURE(prints_schema_with_no_relation) {
  model_t<files_schema_t>::print_schema();
}

FIXTURE(prints_schema_with_relation) {
  model_t<file_parts_schema_t>::print_schema();
}

FIXTURE(builds_a_query) {
  query_builder_t builder;
  builder.select<files_schema_t>()
    .where("file.aws_key is not null")
    .order("file.created_at desc")
    .join<file_parts_schema_t, files_schema_t>("file", "id");
  std::cout << builder.to_string() << std::endl;
}

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}
