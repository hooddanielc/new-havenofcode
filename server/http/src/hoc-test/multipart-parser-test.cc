#include <lick/lick.h>
#include <string>
#include <sstream>
#include <hoc/parsers/multipart.h>


const char *test_str = R"(
------WebKitFormBoundaryLRCVqhSMNAiGzh3L
Content-Disposition: form-data; name="blob"; filename="blob"
Content-Type: application/octet-stream

5f6989b689adc7e407f9eb652349b5fd  How_to_launch_Remix_OS_for_PC.txt
883ab43896cd148b92271f5750427a79  Remix_OS_for_PC_Android_M_64bit_B2016112101.iso
2ec67008882921c270f2ed870d15eed3  Remix_OS_for_PC_Installation_Tool-B2016080802.exe

------WebKitFormBoundaryLRCVqhSMNAiGzh3L
Content-Disposition: form-data; name="blob2"; filename="blob"
Content-Type: application/octet-stream

5f6989b689adc7e407f9eb652349b5fd  How_to_launch_Remix_OS_for_PC.txt
883ab43896cd148b92271f5750427a79  Remix_OS_for_PC_Android_M_64bit_B2016112101.iso
2ec67008882921c270f2ed870d15eed3  Remix_OS_for_PC_Installation_Tool-B2016080802.exe

------WebKitFormBoundaryLRCVqhSMNAiGzh3L
Content-Disposition: form-data; name="regularData"

just a test
------WebKitFormBoundaryLRCVqhSMNAiGzh3L
Content-Disposition: form-data; name="somejson"

{"hey":"girl","how":"it goin"}
------WebKitFormBoundaryLRCVqhSMNAiGzh3L--
)";

using namespace std;
using namespace hoc;

FIXTURE(constructs_with_various_types) {
  const char *boundary = "im-a-boundary";
  std::string expected("--im-a-boundary");
  multipart_parser_t regular(boundary);
  multipart_binary_parser_t binary(boundary);
  EXPECT_EQ(regular.boundary, expected);
  EXPECT_EQ(binary.boundary, expected);
}

FIXTURE(calls_back) {
  const char *boundary = "----WebKitFormBoundaryLRCVqhSMNAiGzh3L";
  multipart_parser_t subject(boundary);

  map<string, string> expected({
    {"Content-Disposition", "form-data; name=\"blob\"; filename=\"blob\""},
    {"Content-Type", "application/octet-stream"},
    {"Content-Disposition", "form-data; name=\"blob2\"; filename=\"blob\""},
    {"Content-Type", "application/octet-stream"},
    {"Content-Disposition", "form-data; name=\"regularData\""},
    {"Content-Disposition", "form-data; name=\"somejson\""}
  });

  vector<string> expected_bodies({
    "5f6989b689adc7e407f9eb652349b5fd  How_to_launch_Remix_OS_for_PC.txt\n"
    "883ab43896cd148b92271f5750427a79  Remix_OS_for_PC_Android_M_64bit_B2016112101.iso\n"
    "2ec67008882921c270f2ed870d15eed3  Remix_OS_for_PC_Installation_Tool-B2016080802.exe\n",
    "5f6989b689adc7e407f9eb652349b5fd  How_to_launch_Remix_OS_for_PC.txt\n"
    "883ab43896cd148b92271f5750427a79  Remix_OS_for_PC_Android_M_64bit_B2016112101.iso\n"
    "2ec67008882921c270f2ed870d15eed3  Remix_OS_for_PC_Installation_Tool-B2016080802.exe\n",
    "just a test",
    "{\"hey\":\"girl\",\"how\":\"it goin\"}"
  });

  std::map<string, string> result;
  subject.on_header([&result](const map<string, string> &headers) {
    result.insert(headers.begin(), headers.end());
  });

  vector<string> result_bodies;
  subject.on_body([&result_bodies](const std::string &body) {
    result_bodies.push_back(body);
  });

  bool ended = false;

  subject.on_multipart_end([&]() {
    ended = true;
    auto it_result = result.begin();
    for (auto it = expected.begin(); it != expected.end(); ++it) {
      EXPECT_EQ(it->first, it_result->first);
      EXPECT_EQ(it->second, it_result->second);
      ++it_result;
    }
    auto it_body_result = result_bodies.begin();
    for (auto it = expected_bodies.begin(); it != expected_bodies.end(); ++it) {
      EXPECT_EQ(*it, *it_body_result);
      ++it_body_result;
    }
  });

  test_str >> subject;
  EXPECT_EQ(ended, true);
}

FIXTURE(binary_parser_works) {
  const char *boundary = "----WebKitFormBoundaryLRCVqhSMNAiGzh3L";
  multipart_binary_parser_t subject(boundary);
  bool header_called = false;
  subject.on_header([&](const map<std::vector<uint8_t>, std::vector<uint8_t>> &) {
    header_called = true;
  });
  bool body_called = false;
  subject.on_body([&](const std::vector<uint8_t> &) {
    body_called = true;
  });

  std::vector<uint8_t> data(test_str, test_str + strlen(test_str));
  data >> subject;
  EXPECT_EQ(body_called, true);
  EXPECT_EQ(header_called, true);
}

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}
