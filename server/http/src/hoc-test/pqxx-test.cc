#include <hoc-test/test-helper.h>

#include <pqxx/pqxx>
#include <sstream>
#include <hoc/env.h>

using namespace hoc;
using namespace std;

FIXTURE(env_vars) {
  EXPECT_EQ(env_t::get().db_name.get(), "hoc_test");
  EXPECT_EQ(env_t::get().db_host.get(), "hoc-db");
  EXPECT_EQ(env_t::get().db_user.get(), "admin_test");
  EXPECT_EQ(env_t::get().db_pass.get(), "123123");
}

std::string con_str() {
  stringstream ss;
  ss << "host=" << env_t::get().db_host.get() << " " <<
    "dbname=" << env_t::get().db_name.get() << " " <<
    "user=" << env_t::get().db_user.get() << " " <<
    "password=" << env_t::get().db_pass.get() << " " <<
    "connect_timeout=10";

  return ss.str();
}

FIXTURE(basic_query_int) {
  EXPECT_OK([]() {
    pqxx::connection c(con_str());
    pqxx::work w(c);
    pqxx::result r = w.exec("SELECT 1");
    EXPECT_EQ(r[0][0].as<int>(), 1);
  });
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
