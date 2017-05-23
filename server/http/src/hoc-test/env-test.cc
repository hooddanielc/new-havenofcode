#include <hoc-test/test-helper.h>

#include <string>
#include <hoc/util.h>
#include <hoc/env.h>

using namespace std;
using namespace hoc;

env_var_t environment_in_datasegment("TESTING", "123");
env_var_t existing_env_in_datasegment("HOC_DB_NAME");

FIXTURE(environment_utils) {
  EXPECT_EQ(environment_in_datasegment.get(), "123");
  EXPECT_EQ(existing_env_in_datasegment.get(), "hoc_dev");
}

FIXTURE(environment_variables) {
  EXPECT_EQ(env_t::get().host.get(), "havenofcode.com");
  EXPECT_EQ(env_t::get().db_name.get(), "hoc_test");
  EXPECT_EQ(env_t::get().db_user.get(), "admin_test");
  EXPECT_EQ(env_t::get().db_pass.get(), "123123");
  EXPECT_EQ(env_t::get().db_host.get(), "hoc-db");
  EXPECT_EQ(env_t::get().google_api_client_id.get(), "XXXXXXXX");
  EXPECT_EQ(env_t::get().google_api_client_secret.get(), "XXXXXXXX");
  EXPECT_EQ(env_t::get().no_reply_email.get(), "noreply@havenofcode.com");
  EXPECT_EQ(env_t::get().anonymous_user.get(), "anonymous");
  EXPECT_EQ(env_t::get().anonymous_pass.get(), "password");
  EXPECT_EQ(env_t::get().aws_key.get(), "XXXXXXXX");
  EXPECT_EQ(env_t::get().aws_secret.get(), "XXXXXXXX");
  EXPECT_EQ(env_t::get().mock_s3_uploads.get(), "1");
  EXPECT_EQ(env_t::get().upload_buffer_size.get(), "16000");
}

int main(int argc, char *argv[]) {
  return test_main(argc, argv);
}
