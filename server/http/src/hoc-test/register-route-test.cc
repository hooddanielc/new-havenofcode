#include <functional>
#include <memory>
#include <string>
#include <hoc-db/db.h>
#include <lick/lick.h>
#include <hoc/json.h>
#include <hoc/app.h>
#include <hoc/routes/register.h>

using namespace hoc;
using namespace std;

string last_email_set;
string last_secret_set;

void app_t::send_registration_email(const std::string &email, const std::string &secret) {
  last_email_set = email;
  last_secret_set = secret;
}

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
  string email("test@test.com");
  vector<db_param_t> insert_user_params({ email, true });
  db.exec("INSERT INTO \"user\" (email, active) VALUES ($1, $2)", insert_user_params);
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
  string email("hood.danielc@gmail.com");
  vector<db_param_t> insert_user_params({ email });

  auto user = db.exec(
    "INSERT INTO \"user\" (email) VALUES ($1)"
    "RETURNING id",
    insert_user_params
  );

  int user_id = user[0][0].int_val();
  string asdf("asdf");
  vector<db_param_t> insert_registration_params({
    db_param_t(user_id),
    asdf,
    asdf,
    asdf
  });

  auto to_delete = db.exec(
    "INSERT INTO registration (\"user\", \"rsaPubD\", \"rsaPubN\", \"secret\")"
    "VALUES ($1, $2, $3, $4)"
    "RETURNING id",
    insert_registration_params
  );

  db.exec("END");

  auto json = dj::json_t::from_string("{\"user\": {}}");
  json["user"]["email"] = "hood.danielc@gmail.com";
  json["user"]["password"] = "password";
  auto fixture = make_request();
  fixture.data_func(json.to_string());
  fixture.end_func();

  db.exec("BEGIN");
  int delete_id = to_delete[0][0].int_val();
  vector<db_param_t> delete_user_params({ db_param_t(delete_id) });
  auto deleted = db.exec(
    "SELECT deleted FROM registration WHERE id = $1",
    delete_user_params
  );
  int non_delete_id = user[0][0].int_val();
  vector<db_param_t> non_deleted_params({ db_param_t(non_delete_id), db_param_t(false) });
  auto nondeleted = db.exec(
    "SELECT deleted, \"rsaPubD\", \"rsaPubN\", secret "
    "FROM registration WHERE \"user\" = $1 AND deleted = $2",
    non_deleted_params
  );
  db.exec("END");

  // decrypt the encrypted message
  mpz_class decrypted;
  mpz_class d(nondeleted[0][1].data());
  mpz_class n(nondeleted[0][2].data());
  mpz_class encrypted(last_secret_set);
  mpz_class secret_message(nondeleted[0][3].data());

  mpz_powm(
    decrypted.get_mpz_t(),
    encrypted.get_mpz_t(),
    d.get_mpz_t(),
    n.get_mpz_t()
  );

  EXPECT_EQ(deleted[0][0].bool_val(), true);
  EXPECT_EQ(nondeleted[0][0].bool_val(), false);
  EXPECT_GT(nondeleted[0][1].size(), static_cast<unsigned int>(10));
  EXPECT_GT(nondeleted[0][2].size(), static_cast<unsigned int>(10));
  EXPECT_GT(nondeleted[0][3].size(), static_cast<unsigned int>(10));
  EXPECT_EQ(decrypted, secret_message);
}

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}