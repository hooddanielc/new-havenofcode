#pragma once

#include <sstream>
#include <memory>
#include <functional>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>
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
  char *data
);

void complete_aws_multipart_upload(
  const std::string &aws_region,
  const std::string &aws_bucket,
  const std::string &aws_key,
  const std::string &upload_id,
  std::vector<std::string> &keys
);

std::string create_upload_promise(
  std::shared_ptr<pqxx::connection> db,
  const std::string &name,
  int64_t bits,
  const std::function<std::string(const std::string &, const std::string &, const std::string &)> &fn = create_aws_multipart_upload
);

void cancel_upload_promise(
  std::shared_ptr<pqxx::connection> db,
  const std::string &id,
  const std::function<void(const std::string &, const std::string &, const std::string &, const std::string &)> &fn = cancel_aws_multipart_upload
);

void complete_file_part_promise(
  std::shared_ptr<pqxx::connection> db,
  const std::string &file_part_id,
  char *data,
  const std::function<std::string(const std::string &, const std::string &, const std::string &, const std::string &, char *)> &fn = complete_aws_file_part_promise
);

void complete_upload_promise(
  std::shared_ptr<pqxx::connection> db,
  const std::string &id,
  const std::function<void(const std::string &, const std::string &, const std::string &, const std::string &, std::vector<std::string> &)> &fn = complete_aws_multipart_upload
);

} // actions
} // hoc
