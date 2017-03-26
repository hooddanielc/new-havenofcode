#include <string>
#include <lick/lick.h>
#include <hoc/json.h>
#include <hoc/db/connection.h>
#include <hoc/actions/account.h>

using namespace std;
using namespace hoc;

void delete_test_registration() {
  auto c = db::admin_connection();
  pqxx::work w(*c);
  w.exec("delete from registration where id is not null");
  w.commit();
}

void delete_test_account() {
  auto c = db::admin_connection();
  pqxx::work w(*c);
  w.exec("delete from account where id is not null");
  w.exec("drop user if exists" + w.quote_name("test@test.com"));
  w.commit();
}

FIXTURE(anonymous_connection) {
  auto c = db::anonymous_connection();
  pqxx::work w(*c);
  pqxx::result r = w.exec("select current_user");
  EXPECT_EQ(r[0][0].as<string>(), "anonymous");
}

FIXTURE(admin_connection) {
  auto c = db::admin_connection();
  pqxx::work w(*c);
  pqxx::result r = w.exec("select current_user");
  EXPECT_EQ(r[0][0].as<string>(), "admin_dev");
}

FIXTURE(register_account_success) {
  actions::register_account("test@test.com", "password");
  delete_test_registration();
}

FIXTURE(register_account_fail_duplicate) {
  actions::register_account("test@test.com", "password");
  bool failed = false;

  try {
    actions::register_account("test@test.com", "password");
  } catch (const std::exception &e) {
    failed = true;
  }

  EXPECT_EQ(failed, true);
  delete_test_registration();
}

FIXTURE(register_then_confirm) {
  try {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
  } catch (const std::exception &e) {
    cout << e.what() << endl;
  }

  delete_test_registration();
  delete_test_account();
}

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}
