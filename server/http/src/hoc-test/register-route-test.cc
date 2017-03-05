#include <functional>
#include <memory>
#include <string>
#include <hoc-db/db.h>
#include <lick/lick.h>
#include <hoc/json.h>
#include <hoc/routes/register.h>

using namespace hoc;
using namespace std;

class request_fixture {
  public:
    mutable std::function<void(const std::string &)> data_func;
    mutable std::function<void()> end_func;
    mutable int status;
    mutable string body_sent;

    request_fixture() {}
    void on_data(const std::function<void(const std::string &)> &fn) const {
      data_func = move(fn);
    }

    void on_end(const std::function<void()> &fn) const {
      end_func = move(fn);
    }

    void set_status(int s) const { status = s; }
    void send_header(const std::string &, const std::string &) const {}
    void set_content_length(int) const {}

    void send_body(const std::string &data) const {
      body_sent.append(data);
    }
};

void refresh_db() {
  db_t db;
  db.exec("BEGIN");
  db.exec("DELETE FROM article WHERE id > 0");
  db.exec("DELETE FROM registration WHERE id > 0");
  db.exec("DELETE FROM \"user\" WHERE id > 0");
  db.exec("END");
}

request_fixture make_request() {
  unique_ptr<route_t<request_fixture>> route(new register_route_t<request_fixture>());
  request_fixture fixture;
  url_match_result_t match;
  route->post(fixture, match);
  return fixture;
}

FIXTURE(fails_if_invalid_json) {
  auto fixture = make_request();
  fixture.data_func("asdfasdf sdf: sdfjjj123 <>");
  fixture.end_func();
  auto response = dj::json_t::from_string(fixture.body_sent.c_str());
  EXPECT_EQ(true, response["error"].as<bool>());
  EXPECT_EQ("invalid json", response["message"].as<string>());
}

FIXTURE(invalid_params) {
  auto json = dj::json_t::from_string("{\"user\": {}}");
  json["user"]["email"] = "test@test.com";
  auto fixture = make_request();
  fixture.data_func(json.to_string());
  fixture.end_func();
  auto response = dj::json_t::from_string(fixture.body_sent.c_str());
  EXPECT_EQ(true, response["error"].as<bool>());
  EXPECT_EQ("invalid json", response["message"].as<string>());
}

FIXTURE(user_already_registered) {
  refresh_db();
  db_t db;
  db.exec("BEGIN");
  db.exec("INSERT INTO \"user\" (email, active) VALUES ($1, $2)", vector<db_param_t>({
    "test@test.com",
    true
  }));
  db.exec("END");

  auto json = dj::json_t::from_string("{\"user\": {}}");
  json["user"]["email"] = "test@test.com";
  json["user"]["password"] = "password";
  auto fixture = make_request();
  fixture.data_func(json.to_string());
  fixture.end_func();
  auto response = dj::json_t::from_string(fixture.body_sent.c_str());
  EXPECT_EQ(response["error"].as<bool>(), true);
  EXPECT_EQ(response["message"].as<string>(), "user active");
}

FIXTURE(deletes_pending_registration) {
  refresh_db();
  db_t db;
  db.exec("BEGIN");

  auto user = db.exec(
    "INSERT INTO \"user\" (email) VALUES ($1)"
    "RETURNING id",
    vector<db_param_t>({
      "test@test.com"
    })
  );

  auto to_delete = db.exec(
    "INSERT INTO registration (\"user\", \"rsaPubD\", \"rsaPubN\", \"secret\")"
    "VALUES ($1, $2, $3, $4)"
    "RETURNING id",
    vector<db_param_t>({
      user[0][0].int_val(),
      "asdf",
      "asdf",
      "asdf"
    })
  );

  db.exec("END");

  auto json = dj::json_t::from_string("{\"user\": {}}");
  json["user"]["email"] = "test@test.com";
  json["user"]["password"] = "password";
  auto fixture = make_request();
  fixture.data_func(json.to_string());
  fixture.end_func();

  db.exec("BEGIN");
  auto deleted = db.exec(
    "SELECT deleted FROM registration WHERE id = $1",
    vector<db_param_t>({ to_delete[0][0].int_val() })
  );
  auto nondeleted = db.exec(
    "SELECT deleted, \"rsaPubD\", \"rsaPubN\", secret "
    "FROM registration WHERE \"user\" = $1 AND deleted = $2",
    vector<db_param_t>({ user[0][0].int_val(), false })
  );
  db.exec("END");

  EXPECT_EQ(deleted[0][0].bool_val(), true);
  EXPECT_EQ(nondeleted[0][0].bool_val(), false);
  EXPECT_GT(nondeleted[0][1].size(), static_cast<unsigned int>(10));
  EXPECT_GT(nondeleted[0][2].size(), static_cast<unsigned int>(10));
  EXPECT_GT(nondeleted[0][3].size(), static_cast<unsigned int>(10));
}

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}