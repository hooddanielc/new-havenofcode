/**
 * Create an s3 multipart upload request and return id
 */

#include <iostream>
#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/Bucket.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/model/CreateMultipartUploadRequest.h>
#include <hoc/env.h>

using namespace std;
using namespace hoc;

int main(int, char *[]) {
  Aws::SDKOptions options;
  Aws::InitAPI(options);
  Aws::Client::ClientConfiguration config;
  config.region = "us-west-2";

  Aws::S3::S3Client client(Aws::Auth::AWSCredentials(
    env_t::get().aws_key,
    env_t::get().aws_secret
  ), config);

  Aws::S3::Model::CreateMultipartUploadRequest request;
  request.SetBucket("havenofcode");
  request.SetKey("example_multipart_key");
  request.SetContentType("text/plain");

  auto request_outcome = client.CreateMultipartUpload(request);

  if (request_outcome.IsSuccess()) {
    cout << "upload id: " << request_outcome.GetResult().GetUploadId() << endl;
  } else {
    cout << "failure: " << request_outcome.GetError().GetMessage() << endl;
  }

  Aws::ShutdownAPI(options);
  return 0;
}
