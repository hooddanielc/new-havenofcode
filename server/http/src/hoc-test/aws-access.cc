#include <test-helper.h>

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/Bucket.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <hoc/env.h>

using namespace std;
using namespace hoc;

FIXTURE(aws_initializes) {
  Aws::SDKOptions options;
  Aws::InitAPI(options);
  Aws::ShutdownAPI(options);
}

FIXTURE(aws_env_vars) {
  EXPECT_EQ(env_t::get().aws_key.get().size(), size_t(20));
  EXPECT_EQ(env_t::get().aws_secret.get().size(), size_t(40));
}

FIXTURE(aws_s3_client_init) {
  Aws::SDKOptions options;
  Aws::InitAPI(options);

  Aws::S3::S3Client s3_client(Aws::Auth::AWSCredentials(
    env_t::get().aws_key,
    env_t::get().aws_secret
  ));

  Aws::ShutdownAPI(options);
}


int main(int argc, char *argv[]) {
  return test_main(argc, argv);
}
