#include <iostream>
#include <lick/lick.h>
#include <hoc/session.h>

using namespace hoc;
using namespace std;

class fixture_req_t {
  public:
    map<string, vector<string>> request_headers;
    map<string, string> sent_headers;
    map<string, vector<string>> mock_cookies;
    int status;
    fixture_req_t() { status = 0; };

    map<string, vector<string>> cookies() {
      return mock_cookies;
    }

    std::string ip() {
      return "someip";
    }

    void send_header(const std::string &header_key, const std::string &header_value) {
      sent_headers[header_key] = header_value;
    }

    void set_status(int stat) {
      status = stat;
    }
};

// creates a new session

void refresh_db() {
  db_t db;
  db.exec("BEGIN");
  db.exec("DELETE FROM session_ip_log WHERE id IS NOT NULL");
  db.exec("DELETE FROM hoc_session WHERE id IS NOT NULL");
  db.exec("DELETE FROM article WHERE id IS NOT NULL");
  db.exec("DELETE FROM \"user\" WHERE id IS NOT NULL");

  auto user = db.exec("SELECT 1 FROM pg_roles WHERE rolname='test@test.com'");

  if (user.rows()) {
    db.exec("REVOKE ALL PRIVILEGES ON ALL TABLES IN SCHEMA public FROM \"test@test.com\"");
    db.exec("DROP USER IF EXISTS \"test@test.com\"");
  }

  db.exec("CREATE USER \"test@test.com\" WITH PASSWORD '123'");\
  db.exec("GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO \"test@test.com\"");
  db.exec("INSERT INTO \"user\" (email) VALUES ('test@test.com')");
  db.exec("END");
}

FIXTURE(creates_session) {
  refresh_db();
  fixture_req_t req;
  req.request_headers["User-Agent"].push_back("some agent");
  auto session = session_t<fixture_req_t>::make(req);
  session.db.exec("BEGIN");
  auto sessions = session.db.exec("SELECT id, \"user\" FROM hoc_session");
  auto logs = session.db.exec("SELECT ip, \"userAgent\", session FROM session_ip_log");
  session.db.exec("END");

  string expected_cookie = "session=";
  expected_cookie += string(sessions[0][0].data());
  expected_cookie += "; Expires=Wed, 21 Oct 2030 07:28:00 GMT; Path=/";
  EXPECT_EQ(req.sent_headers["Set-Cookie"], expected_cookie);
  EXPECT_EQ(session.has_identity(), false);
  EXPECT_EQ(sessions.rows(), 1);
  EXPECT_EQ(logs.rows(), 1);
  EXPECT_EQ(string(sessions[0][0].data()).size(), 36);
  EXPECT_EQ(sessions[0][1].empty(), true);
  EXPECT_EQ(string(logs[0][0].data()), "someip");
  EXPECT_EQ(string(logs[0][1].data()), "some agent");
  EXPECT_EQ(string(sessions[0][0].data()), string(logs[0][2].data()));
}

FIXTURE(connects_with_existing_session_with_no_identity) {
  refresh_db();
  db_t db;
  db.exec("BEGIN");
  auto create_session_result = db.exec(
    "INSERT INTO hoc_session (id) "
    "VALUES (uuid_generate_v4()) "
    "RETURNING id"
  );

  string session_id(create_session_result[0][0].data());

  string ip("someip");
  string agent("some agent");

  std::vector<db_param_t> create_log_params({
    session_id,
    ip,
    agent
  });

  db.exec(
    "INSERT INTO session_ip_log "
    "(id, session, ip, \"userAgent\") "
    "VALUES (uuid_generate_v4(), $1, $2, $3)",
    create_log_params
  );
  db.exec("END");

  fixture_req_t req;
  req.request_headers["User-Agent"].push_back("some agent");
  req.mock_cookies["session"].push_back(session_id);
  auto session = session_t<fixture_req_t>::make(req);
  
  auto all_ips = session.db.exec("SELECT * FROM session_ip_log");
  EXPECT_EQ(all_ips.rows(), 1);
  EXPECT_EQ(session.id, session_id);
  EXPECT_EQ(session.has_identity(), false);
}

FIXTURE(connects_with_existing_session_with_no_identity_changed_ip) {
  refresh_db();
  db_t db;
  db.exec("BEGIN");
  auto create_session_result = db.exec(
    "INSERT INTO hoc_session (id) "
    "VALUES (uuid_generate_v4()) "
    "RETURNING id"
  );

  string session_id(create_session_result[0][0].data());
  string ip("some other ip");
  string agent("some agent");

  std::vector<db_param_t> create_log_params({
    session_id,
    ip,
    agent
  });

  db.exec(
    "INSERT INTO session_ip_log "
    "(id, session, ip, \"userAgent\") "
    "VALUES (uuid_generate_v4(), $1, $2, $3)",
    create_log_params
  );
  db.exec("END");

  fixture_req_t req;
  req.request_headers["User-Agent"].push_back("some agent");
  req.mock_cookies["session"].push_back(session_id);
  auto session = session_t<fixture_req_t>::make(req);
  auto all_ips = session.db.exec("SELECT * FROM session_ip_log");
  EXPECT_EQ(all_ips.rows(), 2);
  EXPECT_EQ(session.id, session_id);
  EXPECT_EQ(session.has_identity(), false);
}

FIXTURE(connects_with_existing_session_with_logged_in_user) {
  refresh_db();
  db_t db;

  db.exec("BEGIN");
  auto select_user = db.exec("SELECT id FROM \"user\" WHERE email = 'test@test.com'");
  int user_id = atoi(select_user[0][0].data());

  auto create_session_result = db.exec(
    "INSERT INTO hoc_session (id) "
    "VALUES (uuid_generate_v4()) "
    "RETURNING id"
  );

  vector<db_param_t> update_session_params({
    db_param_t(user_id),
    create_session_result[0][0].data()
  });

  db.exec("UPDATE hoc_session SET \"user\" = $1 WHERE id = $2", update_session_params);
  string session_id(create_session_result[0][0].data());

  std::vector<db_param_t> create_log_params({
    session_id,
    "someip",
    "some agent"
  });

  db.exec(
    "INSERT INTO session_ip_log "
    "(id, session, ip, \"userAgent\") "
    "VALUES (uuid_generate_v4(), $1, $2, $3)",
    create_log_params
  );
  db.exec("END");

  // create the session
  fixture_req_t req;
  req.request_headers["User-Agent"].push_back("some agent");
  req.mock_cookies["session"].push_back(session_id);
  auto session = session_t<fixture_req_t>::make(req);

  auto current_user = session.db.exec("SELECT SESSION_USER, CURRENT_USER");
  EXPECT_EQ(string(current_user[0][0].data()), "test@test.com");
  EXPECT_EQ(string(current_user[0][1].data()), "test@test.com");
  EXPECT_EQ(session.id, session_id);
}

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}
