#pragma once

#include <sstream>
#include <memory>
#include <functional>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <pqxx/pqxx>
#include <hoc/env.h>

namespace hoc {
namespace actions {

std::string create_aws_multipart_upload();
void cancel_aws_multipart_upload();
void complete_aws_multipart_upload();

std::string create_upload_promise(
  std::shared_ptr<pqxx::connection> db,
  const std::string &name,
  int64_t bits,
  const std::function<std::string()> &fn = create_aws_multipart_upload
);

void cancel_upload_promise(
  std::shared_ptr<pqxx::connection> db,
  const std::string &id,
  const std::function<void()> &fn = cancel_aws_multipart_upload
);

void complete_upload_promise(
  std::shared_ptr<pqxx::connection> db,
  const std::string &id,
  const std::function<void()> &fn = complete_aws_multipart_upload
);

} // actions
} // hoc
