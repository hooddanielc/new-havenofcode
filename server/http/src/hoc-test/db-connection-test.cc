#include <hoc-test/test-helper.h>

#include <string>
#include <functional>
#include <hoc/json.h>

using namespace std;
using namespace hoc;

FIXTURE(anonymous_connection) {
  auto c = db::anonymous_connection();
  pqxx::work w(*c);
  pqxx::result r = w.exec("select current_user");
  EXPECT_EQ(r[0][0].as<string>(), "anonymous");
}

FIXTURE(admin_connection) {
  auto c = db::super_user_connection();
  pqxx::work w(*c);
  pqxx::result r = w.exec("select current_user");
  EXPECT_EQ(r[0][0].as<string>(), "admin_dev");
}

FIXTURE(member_connection_fails) {
  EXPECT_FAIL([]() {
    db::member_connection("asdf", "asdf");
  });
}

FIXTURE(register_account_success) {
  EXPECT_OK([]() {
    actions::register_account("test@test.com", "password");
  });

  delete_all_user_data();
}

FIXTURE(register_account_update_already_registered) {
  EXPECT_OK([]() {
    auto c = db::super_user_connection();
    pqxx::work w(*c);
    actions::register_account("test@test.com", "password");
    auto r1 = w.exec("select salt from registration where email = 'test@test.com'");
    actions::register_account("test@test.com", "password");
    auto r2 = w.exec("select salt from registration where email = 'test@test.com'");
    EXPECT_NE(r1[0][0].as<string>(), r2[0][0].as<string>());
  });

  delete_all_user_data();
}

FIXTURE(register_account_collision) {
  EXPECT_OK([]() {
    actions::register_account("test@test.com", "password");
    actions::register_account("test_another@test.com", "password");
    actions::confirm_account("test_another@test.com", "password");

    auto c = db::super_user_connection();
    pqxx::work w(*c);
    auto r = w.exec("select verified, email from registration where verified = 'TRUE' and (email = 'test@test.com' or email = 'test_another@test.com')");
    EXPECT_EQ(r.size(), size_t(1));
    EXPECT_EQ(r[0][1].as<string>(), "test_another@test.com");
  });

  delete_all_user_data();
}

FIXTURE(register_then_confirm) {
  EXPECT_OK([]() {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
  });

  delete_all_user_data();
}

FIXTURE(member_connection) {
  EXPECT_OK([]() {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
    db::member_connection("test@test.com", "password");
  });

  delete_all_user_data();
}

FIXTURE(member_connection_changes_every_connection) {
  EXPECT_OK([]() {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
    auto c = db::member_connection("test@test.com", "password");
    pqxx::work w(*c);
    auto r_old_hash = w.exec("select salt from account where email = 'test@test.com'");
    auto second_c = db::member_connection("test@test.com", "password");
    pqxx::work second_w(*second_c);
    auto r_new_hash = second_w.exec("select salt from account where email = 'test@test.com'");
    EXPECT_NE(r_new_hash[0][0].as<string>(), r_old_hash[0][0].as<string>());
  });

  delete_all_user_data();
}

FIXTURE(login_action) {
  EXPECT_OK([]() {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
    auto r = actions::login("test@test.com", "password", "some_ip", "some_agent");
    auto c = db::member_connection("test@test.com", "password");
    pqxx::work w(*c);
    auto r_current_id = w.exec("select current_account_id()");
    auto r_session = w.exec("select id, deleted, ip, user_agent from session where created_by=current_account_id()");
    EXPECT_EQ(r[0][1].as<string>(), r_current_id[0][0].as<string>());
    EXPECT_EQ(r[0][0].as<string>(), r_session[0][0].as<string>());
    EXPECT_EQ(r_session[0][1].as<bool>(), false);
    EXPECT_EQ(r_session[0][2].as<string>(), "some_ip");
    EXPECT_EQ(r_session[0][3].as<string>(), "some_agent");
  });

  delete_all_user_data();
}

FIXTURE(session_connection) {
  EXPECT_OK([]() {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
    auto r_session = actions::login("test@test.com", "password", "some_ip", "some_agent");
    auto c = db::session_connection(r_session[0][0].as<string>());
    pqxx::work w(*c);
    auto r_current_user = w.exec("select current_user");
    EXPECT_EQ(r_current_user[0][0].as<string>(), "test@test.com");
  });

  delete_all_user_data();
}

FIXTURE(restore_session_action) {
  EXPECT_OK([]() {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
    auto r_session = actions::login("test@test.com", "password", "some_ip", "some_agent");

    auto r_restored_session = actions::restore_session(
      r_session[0][0].as<string>(),
      "some_ip",
      "some_agent"
    );

    EXPECT_EQ(r_restored_session.size(), r_session.size());

    // fails if user agent is different
    EXPECT_FAIL([&r_session]() {
      actions::restore_session(r_session[0][0].as<string>());
    });
  });

  delete_all_user_data();
}

int main(int argc, char *argv[]) {
  return test_main(argc, argv);
}
