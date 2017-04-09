/**
 * Complete s3 multipart upload request given an array of etags
 */

#include <iostream>
#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/Bucket.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/model/CompleteMultipartUploadRequest.h>
#include <hoc/env.h>

using namespace std;
using namespace hoc;

int main(int argc, char *args[]) {
  if (argc < 3) {
    cout << "aws-abort-multipart <upload-id> <etag...>" << endl;
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

  Aws::S3::Model::CompletedMultipartUpload complete_upload;

  for (int i = 0; i < argc - 2; ++i) {
    const char *etag = args[i + 2];
    Aws::S3::Model::CompletedPart completed_part;
    completed_part.SetETag(etag);
    completed_part.SetPartNumber(i + 1);
    complete_upload.AddParts(completed_part);
  }

  Aws::S3::Model::CompleteMultipartUploadRequest complete_upload_request;
  complete_upload_request.SetBucket("havenofcode");
  complete_upload_request.SetKey("example_multipart_key");
  complete_upload_request.SetUploadId(args[1]);
  complete_upload_request.WithMultipartUpload(complete_upload);
  auto request_outcome = client.CompleteMultipartUpload(complete_upload_request);

  if (request_outcome.IsSuccess()) {
    cout << "success: upload completed" << endl;
  } else {
    cout << "failure: " << request_outcome.GetError().GetMessage() << endl;
  }

  Aws::ShutdownAPI(options);
  return 0;
}
