/**
 * Abort an s3 multipart upload request and stop getting charged
 */

#include <iostream>
#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/Bucket.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/model/AbortMultipartUploadRequest.h>
#include <hoc/env.h>

using namespace std;
using namespace hoc;

int main(int argc, char *args[]) {
  if (argc != 2) {
    cout << "aws-abort-multipart <upload-id>" << endl;
    return 0;
  }

  Aws::SDKOptions options;
  Aws::InitAPI(options);
  Aws::Client::ClientConfiguration config;
  config.region = "us-west-2";

  Aws::S3::S3Client client(Aws::Auth::AWSCredentials(
    env_t::get().aws_key,
    env_t::get().aws_secret
  ), config);

  Aws::S3::Model::AbortMultipartUploadRequest request;
  request.SetBucket("havenofcode");
  request.SetKey("example_multipart_key");
  request.SetUploadId(args[1]);

  auto request_outcome = client.AbortMultipartUpload(request);

  if (request_outcome.IsSuccess()) {
    cout << "abort success: " << args[1] << endl;
  } else {
    cout << "failure" << endl;
  }

  Aws::ShutdownAPI(options);
  return 0;
}
