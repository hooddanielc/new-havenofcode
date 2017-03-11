#include <string>
#include <lick/lick.h>
#include <hoc/request.h>

using namespace std;
using namespace hoc;

FIXTURE(get_read_body) {
  request_t req;
  req.set_url("http://localhost:1337/api/echo");
  string result;

  req.on_data([&result](char *data, size_t len) {
    result += string{ data, len };
  });

  req.send();

  EXPECT_EQ(
    result,
    "GET /api/echo HTTP/1.1\n"
    "Accept */*\n"
    "Host localhost:1337\n"
    "\n"
  );
}

FIXTURE(post_write_body) {
  request_t req;
  req.set_url("http://localhost:1337/api/echo");
  req.add_header("Content-Type: application/json");
  req.add_header("Expect:");
  req.add_header("Transfer-Encoding: chunked");
  string result;

  req.on_data([&result](char *data, size_t len) {
    result += string{ data, len };
  });

  req.send("{\"username\":\"xyz\",\"password\":\"xyz\"}");

  EXPECT_EQ(
    result,
    "POST /api/echo HTTP/1.1\n"
    "Accept */*\n"
    "Content-Type application/json\n"
    "Host localhost:1337\n"
    "Transfer-Encoding chunked\n"
    "\n"
    "{\"username\":\"xyz\",\"password\":\"xyz\"}"
  );
}

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}
