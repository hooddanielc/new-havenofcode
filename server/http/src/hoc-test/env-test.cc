#include <lick/lick.h>
#include <string>
#include <hoc/env.h>

using namespace std;
using namespace hoc;

FIXTURE(rsa_crypto_t_init) {
  EXPECT_EQ(string(env_t::get().host), "havenofcode.com");
  EXPECT_EQ(string(env_t::get().db_name), "hoc_dev");
  EXPECT_EQ(string(env_t::get().db_user), "admin_dev");
  EXPECT_EQ(string(env_t::get().db_pass), "123123");
  EXPECT_EQ(string(env_t::get().db_host), "hoc-db");
  EXPECT_EQ(env_t::get().google_api_client_id, "XXXXXXXX");
  EXPECT_EQ(env_t::get().google_api_client_secret, "XXXXXXXX");
  EXPECT_EQ(string(env_t::get().no_reply_email), "noreply@havenofcode.com");
  EXPECT_EQ(string(env_t::get().anonymous_user), "anonymous");
  EXPECT_EQ(string(env_t::get().anonymous_pass), "password");
  EXPECT_EQ(string(env_t::get().aws_key), "XXXXXXXX");
  EXPECT_EQ(string(env_t::get().aws_secret), "XXXXXXXX");
  EXPECT_EQ(env_t::get().mock_s3_uploads, true);
}

int main(int argc, char *argv[]) {
  return dj::lick::main(argc, argv);
}
