#include <lick/lick.h>
#include <vector>
#include <map>
#include <hoc-test/fixtures.h>
#include <hoc/db/connection.h>
#include <hoc/actions/account.h>

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

template <typename req_t>
class session_t {
  private:
    string session_id;
    string account_id;
    bool is_authenticated;

    void create_new_session(req_t &req) {
      db = db::anonymous_connection();
      pqxx::work w(*db);
      stringstream ss;
      auto ip = req.ip();
      auto user_agent = req.user_agent();
      ss << "insert into session (ip, user_agent) values ("
         << w.quote(ip) << ","
         << w.quote(user_agent)
         << ") returning id";
      auto r_for_session = w.exec(ss);
      session_id = r_for_session[0][0].as<string>();
      ss.str(string());
      ss.clear();

      ss << "insert into session_ip_log (ip, user_agent, session) values ("
         << w.quote(ip) << ","
         << w.quote(user_agent) << ","
         << w.quote(r_for_session[0][0].as<string>())
         << ")";

      w.exec(ss);
      w.commit();
      send_session_id(req, r_for_session[0][0].as<string>());
    }

    void send_session_id(req_t &req, const string &id) {
      string session_header = "session=";
      session_header.append(id).append("; Wed, 21 Oct 2030 07:28:00 GMT");
      req.send_header("Set-Cookie", session_header);
    }

    void restore_session(
      req_t &req,
      std::map<std::string, std::vector<std::string>> cookies
    ) {
      auto temp_c = db::super_user_connection();
      pqxx::work w(*temp_c);
      auto session_cookie = cookies["session"][0];
      stringstream ss;
      ss << "select created_by, id,"
         << "user_agent from session where "
         << "id = " << w.quote(session_cookie) << " and "
         << "deleted = 'FALSE'";
      auto r_current_session = w.exec(ss);

      if (r_current_session.size() == 0) {
        create_new_session(req);
      } else {
        auto user_agent = req.user_agent();

        if (user_agent != r_current_session[0][2].as<string>()) {
          ss.str(string());
          ss.clear();
          ss << "udpate session set deleted = 'TRUE' "
             << "where id = " << r_current_session[0][1].as<string>();
          w.exec(ss);
          w.commit();
          throw runtime_error("user agent changed");
        }

        auto ip = req.ip();
        if (r_current_session[0][0].is_null()) {
          w.exec("set role anonymous");
          w.exec("set session authorization anonymous");
        } else {
          ss.str(string());
          ss.clear();
          ss << "select email from account where id = " << w.quote(r_current_session[0][0].as<string>());
          auto r_email = w.exec(ss);
          ss.str(string());
          ss.clear();
          ss << "set role " << w.quote_name(r_email[0][0].as<string>());
          w.exec(ss);
          ss.str(string());
          ss.clear();
          ss << "set session authorization "
             << w.quote_name(r_email[0][0].as<string>());
          w.exec(ss);
          ss.str(string());
          ss.clear();
          ss << "insert into session_ip_log (ip, user_agent, session, created_by) values ("
             << w.quote(ip) << ","
             << w.quote(user_agent) << ","
             << w.quote(r_current_session[0][1].as<string>()) << ","
             << "current_account_id()"
             << ")";
        }

        w.exec(ss);
        ss.str(string());
        ss.clear();
        ss << "update session set updated_at = 'NOW()', "
           << "ip = " << w.quote(ip) << " "
           << "where id = " << w.quote(r_current_session[0][1].as<string>());
        w.exec(ss);
        w.commit();
        session_id = r_current_session[0][1].as<string>();

        if (!r_current_session[0][0].is_null()) {
          is_authenticated = true;
          account_id = r_current_session[0][0].as<string>();
        }

        db = temp_c;
      }
    }

  public:
    shared_ptr<pqxx::connection> db;

    // create an anonymous session
    session_t(req_t &req) : is_authenticated(false) {
      auto cookies = req.cookies();

      if (cookies.count("session") && cookies["session"].size() == 1) {
        restore_session(req, cookies);
      } else {
        create_new_session(req);
      }
    }

    // create session authenticated session
    // using username and password.
    // should throw if connection fails.
    session_t(
      req_t &req,
      const std::string &email,
      const std::string &password
    ) : is_authenticated(true) {
      db = db::member_connection(email, password);
      pqxx::work w(*db);
      auto ip = req.ip();
      auto user_agent = req.user_agent();
      stringstream ss;
      ss << "insert into session (ip, user_agent, created_by) VALUES ("
         << w.quote(ip) << ","
         << w.quote(user_agent) << ","
         << "current_account_id()"
         << ") returning id, created_by";
      auto r_for_session = w.exec(ss);
      session_id = r_for_session[0][0].as<string>();
      account_id = r_for_session[0][1].as<string>();
      ss.str(string());
      ss.clear();
      ss << "insert into session_ip_log (ip, user_agent, session, created_by) values ("
         << w.quote(ip) << ","
         << w.quote(user_agent) << ","
         << w.quote(r_for_session[0][0].as<string>()) << ","
         << "current_account_id()"
         << ")";
      w.exec(ss);
      w.commit();
      send_session_id(req, r_for_session[0][0].as<string>());
    }

    string id() {
      return session_id;
    }

    bool authenticated() {
      return is_authenticated;
    }

    string user_id() {
      return account_id;
    }

    session_t() = delete;

    // should set headers to log user out
    void logout() {

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

  delete_test_accounts();
}

FIXTURE(anonymous_session_cookie_key) {
  EXPECT_OK([]() {
    fixture_req_t req;
    session_t<fixture_req_t> session(req);
    pqxx::work w(*session.db);
    auto r_current_user = w.exec("select current_user");
    EXPECT_EQ(r_current_user[0][0].as<string>(), "anonymous");
    auto r_for_session = w.exec("select id from session");
    EXPECT_EQ(r_for_session.size(), size_t(1));
    EXPECT_EQ(session.authenticated(), false);
    EXPECT_EQ(session.id(), r_for_session[0][0].as<string>());
  });

  delete_test_accounts();
}

FIXTURE(login_session) {
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
    auto r_for_user = w.exec("select created_by from session");
    auto r_for_user_id = w.exec("select current_account_id()");
    EXPECT_EQ(r_for_user[0][0].as<string>(), r_for_user_id[0][0].as<string>());
    EXPECT_EQ(session.authenticated(), true);
    EXPECT_EQ(session.id(), r_session[0][0].as<string>());
    EXPECT_EQ(session.user_id(), r_session[0][1].as<string>());
  });

  delete_test_accounts();
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
    session_t<fixture_req_t> restored_session(req);
    EXPECT_EQ(login_session.id(), restored_session.id());
    EXPECT_EQ(restored_session.authenticated(), true);
    EXPECT_EQ(restored_session.user_id(), r_session[0][1].as<string>());
  });

  delete_test_accounts();
}

FIXTURE(restore_session_user_agent_changed) {
  EXPECT_FAIL([]() {
    actions::register_account("test@test.com", "password");
    actions::confirm_account("test@test.com", "password");
    fixture_req_t req;
    session_t<fixture_req_t> login_session(req, "test@test.com", "password");
    pqxx::work w(*login_session.db);
    auto r_session = w.exec("select id, created_by from session");
    req.set_cookies["session"].push_back(r_session[0][0].as<string>());
    req.set_user_agent = "different";
    session_t<fixture_req_t> restored_session(req);
  });

  delete_test_accounts();
}

FIXTURE(restore_anonymous_session) {
  EXPECT_FAIL([]() {
    fixture_req_t req;
    session_t<fixture_req_t> session(req);
    pqxx::work w(*session.db);
    req.set_cookies["session"].push_back(session.id());
    req.set_user_agent = "different";
    session_t<fixture_req_t> restored_session(req);
    EXPECT_EQ(restored_session.authenticated(), false);
    EXPECT_EQ(restored_session.authenticated(), session.authenticated());
    EXPECT_EQ(session.id(), restored_session.id());
  });

  delete_test_accounts();
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
    session_t<fixture_req_t> restored_session(req);
    EXPECT_NE(restored_session.id(), login_session.id());
    EXPECT_EQ(restored_session.authenticated(), false);
  });

  delete_test_accounts();
}

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}
