/**
 * Append 5MB of data to file with upload ID
 */

#include <iostream>
#include <stdlib.h>
#include <aws/core/Aws.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/Bucket.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/model/UploadPartRequest.h>
#include <hoc/env.h>

using namespace std;
using namespace hoc;

shared_ptr<Aws::StringStream> Create5MbStreamForUploadPart(const char *partTag) {
  uint32_t five_mb_size = 5 * 1024 * 1024;

  Aws::StringStream pattern_stream;
  pattern_stream << "Multi-Part upload Test Part " << partTag << ":" << std::endl;
  Aws::String pattern = pattern_stream.str();

  Aws::String scratch_string;
  scratch_string.reserve(five_mb_size);

  // 5MB is a hard minimum for multi part uploads;
  // make sure the final string is at least that long
  uint32_t pattern_copy_count = static_cast< uint32_t >(five_mb_size / pattern.size() + 1);
  for (uint32_t i = 0; i < pattern_copy_count; ++i) {
    scratch_string.append(pattern);
  }

  auto streamPtr = Aws::MakeShared<Aws::StringStream>("uploadpart", scratch_string);
  streamPtr->seekg(0);
  streamPtr->seekp(0, std::ios_base::end);
  return streamPtr;
}

Aws::S3::Model::UploadPartOutcomeCallable MakeUploadPartOutcomeAndGetCallable(
  unsigned part_number,
  const Aws::Utils::ByteBuffer &md5_of_stream,
  const std::shared_ptr<Aws::IOStream> &part_stream,
  const Aws::String &bucket_name,
  const char *object_name,
  const Aws::String &upload_id,
  const Aws::S3::S3Client &client
) {
  Aws::S3::Model::UploadPartRequest upload_request;
  upload_request.SetBucket(bucket_name);
  upload_request.SetKey(object_name);
  upload_request.SetPartNumber(part_number);
  upload_request.SetUploadId(upload_id);
  upload_request.SetBody(part_stream);
  upload_request.SetContentMD5(Aws::Utils::HashingUtils::Base64Encode(md5_of_stream));

  auto startingPoint = part_stream->tellg();
  part_stream->seekg(0LL, part_stream->end);
  upload_request.SetContentLength(static_cast<long>(part_stream->tellg()));
  part_stream->seekg(startingPoint);

  return client.UploadPartCallable(upload_request);
}

int main(int argc, char *args[]) {
  if (argc != 3) {
    cout << "aws-upload-random-part <upload-id> <part-number>" << endl;
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

  auto part_stream = Create5MbStreamForUploadPart(args[2]);
  Aws::Utils::ByteBuffer part_md5(Aws::Utils::HashingUtils::CalculateMD5(*part_stream));

  auto upload_part_callable = MakeUploadPartOutcomeAndGetCallable(
    atoi(args[2]),
    part_md5,
    part_stream,
    "havenofcode",
    "example_multipart_key",
    args[1],
    client
  );

  auto upload_part_outcome = upload_part_callable.get();

  if (upload_part_outcome.IsSuccess()) {
    cout << "Part Number: " << atoi(args[2]) << endl;
    cout << "Part ETAG: " << upload_part_outcome.GetResult().GetETag() << endl;
  } else {
    cout << "failure: " << upload_part_outcome.GetError().GetMessage() << endl;
  }

  Aws::ShutdownAPI(options);
  return 0;
}
