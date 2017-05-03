#include <string>
#include <lick/lick.h>
#include <hoc/request.h>
#include <hoc/util.h>

using namespace std;
using namespace hoc;

FIXTURE(get_read_body) {
  request_t req;
  req.set_url("http://localhost/api/echo");
  string result;

  req.on_data([&result](char *data, size_t len) {
    result += string{ data, len };
  });

  req.send();
  vector<string> lines;
  split(result, '\n', lines);
  EXPECT_EQ("GET /api/echo HTTP/1.1", lines[0]);
}

FIXTURE(post_write_body) {
  request_t req;
  req.set_url("http://localhost/api/echo");
  req.add_header("Content-Type: application/json");
  req.add_header("Expect:");
  req.add_header("Transfer-Encoding: chunked");
  string result;

  req.on_data([&result](char *data, size_t len) {
    result += string{ data, len };
  });

  req.send("{\"username\":\"xyz\",\"password\":\"xyz\"}");
  vector<string> lines;
  split(result, '\n', lines);
  EXPECT_EQ("POST /api/echo HTTP/1.1", lines[0]);
}

int main(int argc, char *argv[]) {
  request_t::init();
  return dj::lick::main(argc, argv);
}
