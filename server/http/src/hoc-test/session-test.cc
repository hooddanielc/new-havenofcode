#include <lick/lick.h>
#include <vector>
#include <map>
#include <hoc-test/fixtures.h>
#include <hoc/db/connection.h>
#include <hoc/actions/account.h>
#include <hoc/session.h>

using namespace std;
using namespace hoc;

class fixture_req_t {
  public:
    string last_header_key;
    string last_header_value;
    string set_ip;
    string set_user_agent;
    std::map<std::string, std::vector<std::string>> set_cookies;

    fixture_req_t() : 
      last_header_key(""),
      last_header_value(""),
      set_ip("ip"),
      set_user_agent("user_agent"),
      set_cookies({}) {}

    std::map<std::string, std::vector<std::string>> cookies() {
      return set_cookies;
    }

    void send_header(const string &key, const string &value) {
      last_header_key = key;
      last_header_value = value;
    }

    string ip() {
      return set_ip;
    }

    string user_agent() {
      return set_user_agent;
    }
};

FIXTURE(anonymous_session) {
  EXPECT_OK([]() {
    fixture_req_t req;
    session_t<fixture_req_t> session(req);
    pqxx::work w(*session.db);
    auto r_current_user = w.exec("select current_user");
    EXPECT_EQ(r_current_user[0][0].as<string>(), "anonymous");
    EXPECT_EQ(session.id().size(), size_t(36));
    auto r_for_session = w.exec("select count(*) from session");
    EXPECT_EQ(r_for_session.size(), size_t(1));
  });

  delete_all_user_data();
}

FIXTURE(anonymous_session_no_cookie_key) {
  EXPECT_OK([]() {
    fixture_req_t req;
    session_t<fixture_req_t> session(req);
    pqxx::work w(*session.db);
    auto r_current_user = w.exec("select current_user");
    EXPECT_EQ(r_current_user[0][0].as<string>(), "anonymous");
    auto r_for_session = w.exec("select id from session");
    EXPECT_EQ(r_for_session.size(), size_t(1));
    EXPECT_EQ(req.last_header_key, "Set-Cookie");
    EXPECT_EQ(
      req.last_header_value,
      "session=" + r_for_session[0][0].as<string>() + "; Expires=Wed, 21 Oct 2030 07:28:00 GMT; Path=/"
    );
    EXPECT_EQ(session.authenticated(), false);
    EXPECT_EQ(session.id(), r_for_session[0][0].as<string>());
  });

  delete_all_user_data();
}

FIXTURE(construct_authenticated_session) {
  EXPECT_OK([]() {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
    fixture_req_t req;
    session_t<fixture_req_t> session(req, "test@test.com", "password");
    pqxx::work w(*session.db);
    auto r_current_user = w.exec("select current_user");
    EXPECT_EQ(r_current_user[0][0].as<string>(), "test@test.com");
    auto r_session = w.exec("select id, created_by from session");
    EXPECT_EQ(r_session.size(), size_t(1));
    EXPECT_EQ(req.last_header_key, "Set-Cookie");
    EXPECT_EQ(
      req.last_header_value,
      "session=" + r_session[0][0].as<string>() + "; Expires=Wed, 21 Oct 2030 07:28:00 GMT; Path=/"
    );
    auto r_for_user = w.exec("select created_by from session");
    auto r_for_user_id = w.exec("select current_account_id()");
    EXPECT_EQ(r_for_user[0][0].as<string>(), r_for_user_id[0][0].as<string>());
    EXPECT_EQ(session.authenticated(), true);
    EXPECT_EQ(session.id(), r_session[0][0].as<string>());
    EXPECT_EQ(session.user_id(), r_session[0][1].as<string>());
  });

  delete_all_user_data();
}

FIXTURE(anonymous_session_to_authenticated) {
  EXPECT_OK([]() {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
    fixture_req_t req;
    session_t<fixture_req_t> session(req);
    session.login(req, "test@test.com", "password");
    pqxx::work w(*session.db);
    auto r_current_user = w.exec("select current_user");
    EXPECT_EQ(r_current_user[0][0].as<string>(), "test@test.com");
    auto r_session = w.exec("select id, created_by from session where id='" + session.id() + "'");
    EXPECT_EQ(r_session.size(), size_t(1));
    EXPECT_EQ(req.last_header_key, "Set-Cookie");
    EXPECT_EQ(
      req.last_header_value,
      "session=" + r_session[0][0].as<string>() + "; Expires=Wed, 21 Oct 2030 07:28:00 GMT; Path=/"
    );
    auto r_for_user = w.exec("select created_by from session where id='" + session.id() + "'");
    auto r_for_user_id = w.exec("select current_account_id()");
    EXPECT_EQ(r_for_user[0][0].as<string>(), r_for_user_id[0][0].as<string>());
    EXPECT_EQ(session.authenticated(), true);
    EXPECT_EQ(session.id(), r_session[0][0].as<string>());
    EXPECT_EQ(session.user_id(), r_session[0][1].as<string>());
  });

  delete_all_user_data();
}

FIXTURE(restore_session) {
  EXPECT_OK([]() {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
    fixture_req_t req;
    session_t<fixture_req_t> login_session(req, "test@test.com", "password");
    pqxx::work w(*login_session.db);
    auto r_session = w.exec("select id, created_by from session");
    EXPECT_EQ(r_session.size(), size_t(1));
    req.set_cookies["session"].push_back(r_session[0][0].as<string>());
    fixture_req_t another_req;
    session_t<fixture_req_t> restored_session(req);
    EXPECT_EQ(another_req.last_header_key, "");
    EXPECT_EQ(another_req.last_header_value, "");
    EXPECT_EQ(login_session.id(), restored_session.id());
    EXPECT_EQ(restored_session.authenticated(), true);
    EXPECT_EQ(restored_session.user_id(), r_session[0][1].as<string>());
  });

  delete_all_user_data();
}

FIXTURE(restore_session_user_agent_changed) {
  EXPECT_OK([]() {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
    fixture_req_t req;
    session_t<fixture_req_t> login_session(req, "test@test.com", "password");
    pqxx::work w(*login_session.db);
    auto r_session = w.exec("select id, created_by from session");
    req.set_cookies["session"].push_back(r_session[0][0].as<string>());
    req.set_user_agent = "different";
    session_t<fixture_req_t> restored_session(req);
    EXPECT_NE(restored_session.id(), login_session.id());
  });

  delete_all_user_data();
}

FIXTURE(restore_anonymous_session) {
  EXPECT_OK([]() {
    fixture_req_t req;
    session_t<fixture_req_t> session(req);
    pqxx::work w(*session.db);
    req.set_cookies["session"].push_back(session.id());
    req.set_user_agent = "different";
    session_t<fixture_req_t> restored_session(req);
    EXPECT_EQ(req.last_header_key, "Set-Cookie");
    EXPECT_EQ(restored_session.authenticated(), false);
    EXPECT_EQ(restored_session.authenticated(), session.authenticated());
    EXPECT_NE(session.id(), restored_session.id());
  });

  delete_all_user_data();
}

FIXTURE(restoring_deleted_session_returns_new) {
  EXPECT_OK([]() {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
    fixture_req_t req;
    session_t<fixture_req_t> login_session(req, "test@test.com", "password");
    pqxx::work w(*login_session.db);
    w.exec("update session set deleted = 'TRUE' where created_by = current_account_id()");
    w.commit();
    req.set_cookies["session"].push_back(login_session.id());
    fixture_req_t another_req;
    session_t<fixture_req_t> restored_session(another_req);
    EXPECT_EQ(another_req.last_header_key, "Set-Cookie");
    EXPECT_EQ(
      another_req.last_header_value,
      "session=" + restored_session.id() + "; Expires=Wed, 21 Oct 2030 07:28:00 GMT; Path=/"
    );
    EXPECT_NE(restored_session.id(), login_session.id());
    EXPECT_EQ(restored_session.authenticated(), false);
  });

  delete_all_user_data();
}

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}
