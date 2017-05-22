#include <hoc-test/test-helper.h>

#include <pqxx/pqxx>
#include <sstream>

using namespace hoc;
using namespace std;

FIXTURE(env_vars) {
  EXPECT_EQ(string(getenv("HOC_DB_NAME")), "hoc_test");
  EXPECT_EQ(string(getenv("HOC_DB_HOST")), "hoc-db");
  EXPECT_EQ(string(getenv("HOC_DB_USER")), "admin_test");
  EXPECT_EQ(string(getenv("HOC_DB_PASSWORD")), "123123");
}

std::string con_str() {
  return "host=hoc-db dbname=hoc_dev user=admin_dev password=123123 connect_timeout=10";
}

FIXTURE(basic_query_int) {
  pqxx::connection c(con_str());
  pqxx::work w(c);
  pqxx::result r = w.exec("SELECT 1");
  EXPECT_EQ(r[0][0].as<int>(), 1);
}

FIXTURE(basic_query_double) {
  pqxx::connection c(con_str());
  pqxx::work w(c);
  pqxx::result r = w.exec("SELECT 1.23");
  EXPECT_EQ(r[0][0].as<double>(), 1.23);
}

FIXTURE(basic_query_date) {
  pqxx::connection c(con_str());
  pqxx::work w(c);
  pqxx::result r = w.exec("SELECT NOW()");
  EXPECT_GT(r[0][0].as<string>().size(), size_t(0));
}

FIXTURE(basic_query_uuid) {
  pqxx::connection c(con_str());
  pqxx::work w(c);
  pqxx::result r = w.exec("SELECT uuid_generate_v4()");
  EXPECT_EQ(r[0][0].as<string>().size(), size_t(36));
}

FIXTURE(escape_quote_and_string) {
  pqxx::connection c(con_str());
  pqxx::work w(c);
  stringstream ss;
  ss << "SELECT " << w.quote_name("id") << " FROM " << w.quote_name("account") << " "
     << "WHERE " << w.quote_name("name") << " = " << w.quote("abc123");
  pqxx::result r = w.exec(ss);
}

int main(int argc, char *argv[]) {
  return test_main(argc, argv);
}
