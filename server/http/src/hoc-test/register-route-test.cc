#include <functional>
#include <memory>
#include <string>
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
  EXPECT_EQ("true", response["error"].to_string());
  EXPECT_EQ("\"invalid json\"", response["message"].to_string());
}

FIXTURE(invalid_params) {
  auto json = dj::json_t::from_string("{\"user\": {}}");
  json["user"]["email"] = "test@test.com";
  auto fixture = make_request();
  fixture.data_func(json.to_string());
  fixture.end_func();
  auto response = dj::json_t::from_string(fixture.body_sent.c_str());
  EXPECT_EQ("true", response["error"].to_string());
  EXPECT_EQ("\"invalid json\"", response["message"].to_string());
}

FIXTURE(creates_row) {
  // TODO - create table
  // TODO - create email interface
  auto json = dj::json_t::from_string("{\"user\": {}}");
  json["user"]["email"] = "test@test.com";
  json["user"]["password"] = "password";
  auto fixture = make_request();
  fixture.data_func(json.to_string());
  fixture.end_func();
}

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}