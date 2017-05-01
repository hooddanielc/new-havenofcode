#pragma once

#include <cstdlib>
#include <sstream>
#include <memory>
#include <functional>
#include <cmath>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>
#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/Bucket.h>
#include <aws/s3/model/CreateMultipartUploadRequest.h>
#include <aws/s3/model/AbortMultipartUploadRequest.h>
#include <aws/s3/model/CompleteMultipartUploadRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/UploadPartRequest.h>
#include <pqxx/pqxx>
#include <hoc/env.h>

namespace hoc {
namespace actions {

std::string create_aws_multipart_upload(
  const std::string &aws_region,
  const std::string &aws_bucket,
  const std::string &aws_key
);

void cancel_aws_multipart_upload(
  const std::string &aws_region,
  const std::string &aws_bucket,
  const std::string &aws_key,
  const std::string &upload_id
);

std::string complete_aws_file_part_promise(
  const std::string &aws_region,
  const std::string &aws_bucket,
  const std::string &aws_key,
  const std::string &upload_id,
  const int part_number,
  const std::string &file_path
);

void complete_aws_multipart_upload(
  const std::string &aws_region,
  const std::string &aws_bucket,
  const std::string &aws_key,
  const std::string &upload_id,
  const std::vector<std::string> &keys
);

std::string create_upload_promise(
  std::shared_ptr<pqxx::connection> db,
  const std::string &name,
  uint64_t bytes,
  const std::function<std::string(const std::string &, const std::string &, const std::string &)> &fn = create_aws_multipart_upload
);

void cancel_upload_promise(
  std::shared_ptr<pqxx::connection> db,
  const std::string &id,
  const std::function<void(
    const std::string &,
    const std::string &,
    const std::string &,
    const std::string &
  )> &fn = cancel_aws_multipart_upload
);

// start upload promise must be
// called before completing promise
void start_file_part_promise(
  std::shared_ptr<pqxx::connection> db,
  const std::string &id
);

void complete_file_part_promise(
  std::shared_ptr<pqxx::connection> db,
  const std::string &file_part_id,
  const std::string &file_path,
  const std::function<std::string(
    const std::string &,
    const std::string &,
    const std::string &,
    const std::string &,
    const int,
    const std::string &
  )> &fn = complete_aws_file_part_promise
);

void complete_upload_promise(
  std::shared_ptr<pqxx::connection> db,
  const std::string &id,
  const std::function<void(
    const std::string &,
    const std::string &,
    const std::string &,
    const std::string &,
    const std::vector<std::string> &
  )> &fn = complete_aws_multipart_upload
);

} // actions
} // hoc
