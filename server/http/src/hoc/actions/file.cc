#include <hoc/actions/file.h>

using namespace std;

namespace hoc {
namespace actions {

string create_aws_multipart_upload(
  const string &aws_region,
  const string &aws_bucket,
  const string &aws_key
) {
  if (env_t::get().mock_s3_uploads.get() == "1") {
    return "not-uploaded-to-s3";
  }

  Aws::Client::ClientConfiguration config;
  config.region = aws_region.c_str();

  Aws::S3::S3Client client(Aws::Auth::AWSCredentials(
    env_t::get().aws_key.get().c_str(),
    env_t::get().aws_secret.get().c_str()
  ), config);

  Aws::S3::Model::CreateMultipartUploadRequest request;
  request.SetBucket(aws_bucket.c_str());
  request.SetKey(aws_key.c_str());
  auto request_outcome = client.CreateMultipartUpload(request);

  if (!request_outcome.IsSuccess()) {
    throw std::runtime_error(request_outcome.GetError().GetMessage().c_str());
  }

  return request_outcome.GetResult().GetUploadId().c_str();
}

void cancel_aws_multipart_upload(
  const string &aws_region,
  const string &aws_bucket,
  const string &aws_key,
  const string &upload_id
) {
  if (env_t::get().mock_s3_uploads.get() == "1") {
    return;
  }

  Aws::Client::ClientConfiguration config;
  config.region = aws_region.c_str();

  Aws::S3::S3Client client(Aws::Auth::AWSCredentials(
    env_t::get().aws_key.get().c_str(),
    env_t::get().aws_secret.get().c_str()
  ), config);

  Aws::S3::Model::AbortMultipartUploadRequest request;
  request.SetBucket(aws_bucket.c_str());
  request.SetKey(aws_key.c_str());
  request.SetUploadId(upload_id.c_str());
  auto request_outcome = client.AbortMultipartUpload(request);

  if (!request_outcome.IsSuccess()) {
    throw runtime_error(request_outcome.GetError().GetMessage().c_str());
  }
}

std::string mock_complete_aws_file_part_promise(
  const string &aws_key,
  const int part_number,
  const std::string &file_path
) {
  std::string user_dir(aws_key.begin() + 1, aws_key.begin() + 37);
  std::string tmp_path(env_t::get().upload_tmp_path.get());
  std::string new_dir(tmp_path + "/fake-s3/" + user_dir);
  mkdir((tmp_path + "/fake-s3").c_str(), 0644);
  mkdir(new_dir.c_str(), 0644);
  std::ifstream src(file_path, std::ios::binary);
  std::string dst_path = tmp_path + "/fake-s3" + aws_key + "-" + to_string(part_number);
  std::ofstream dst(dst_path, std::ios::binary);
  dst << src.rdbuf();
  return dst_path;
}

string complete_aws_file_part_promise(
  const string &aws_region,
  const string &aws_bucket,
  const string &aws_key,
  const string &upload_id,
  const int part_number,
  const std::string &file_path
) {
  if (env_t::get().mock_s3_uploads.get() == "1") {
    return mock_complete_aws_file_part_promise(
      aws_key,
      part_number,
      file_path
    );
  }

  Aws::Client::ClientConfiguration config;
  config.region = aws_region.c_str();
  Aws::S3::S3Client client(Aws::Auth::AWSCredentials(
    env_t::get().aws_key.get().c_str(),
    env_t::get().aws_secret.get().c_str()
  ), config);

  // empty upload id means the file
  // is less than 5MB. Files less than 5 MB do
  // are not allowed to be uploaded in multiple parts
  if (upload_id == "") {
    Aws::S3::Model::PutObjectRequest object_request;
    object_request.SetBucket(aws_bucket.c_str());
    object_request.SetKey(aws_key.c_str());
    auto input_data = Aws::MakeShared<Aws::FStream>(
      "PutObjectInputStream",
      file_path.c_str(), std::ios_base::in
    );
    object_request.SetBody(input_data);
    auto put_object_outcome = client.PutObject(object_request);

    if (!put_object_outcome.IsSuccess()) {
      throw runtime_error(put_object_outcome.GetError().GetMessage().c_str());
    }

    return "";
  }

  auto part_stream = Aws::MakeShared<Aws::FStream>(
    "PutObjectInputStream",
    file_path.c_str(), std::ios_base::in
  );
  Aws::Utils::ByteBuffer part_md5(Aws::Utils::HashingUtils::CalculateMD5(*part_stream));
  Aws::S3::Model::UploadPartRequest upload_request;
  upload_request.SetBucket(aws_bucket.c_str());
  upload_request.SetKey(aws_key.c_str());
  upload_request.SetPartNumber(part_number);
  upload_request.SetUploadId(upload_id.c_str());
  upload_request.SetBody(part_stream);
  upload_request.SetContentMD5(Aws::Utils::HashingUtils::Base64Encode(part_md5));
  auto upload_part_outcome = client.UploadPart(upload_request);

  if (!upload_part_outcome.IsSuccess()) {
    throw runtime_error(upload_part_outcome.GetError().GetMessage().c_str());
  }

  return upload_part_outcome.GetResult().GetETag().c_str();
}

void mock_complete_aws_multipart_upload(
  const string &aws_key,
  const vector<string> &keys
) {
  std::string tmp_path(env_t::get().upload_tmp_path.get());
  std::string dst_path = tmp_path + "/fake-s3" + aws_key;
  std::ofstream dst(dst_path, std::ios::binary);
  for (auto it = keys.begin(); it != keys.end(); ++it) {
    std::ifstream src(*it, std::ios::binary);
    dst << src.rdbuf();
    unlink((*it).c_str());
  }
}

void complete_aws_multipart_upload(
  const string &aws_region,
  const string &aws_bucket,
  const string &aws_key,
  const string &upload_id,
  const vector<string> &keys
) {
  if (env_t::get().mock_s3_uploads.get() == "1") {
    return mock_complete_aws_multipart_upload(
      aws_key,
      keys
    );
  }

  Aws::Client::ClientConfiguration config;
  config.region = aws_region.c_str();

  Aws::S3::S3Client client(Aws::Auth::AWSCredentials(
    env_t::get().aws_key.get().c_str(),
    env_t::get().aws_secret.get().c_str()
  ), config);

  Aws::S3::Model::CompletedMultipartUpload complete_upload;

  int i = 0;
  for (auto it = keys.begin(); it != keys.end(); ++it) {
    const char *etag = (*it).c_str();
    Aws::S3::Model::CompletedPart completed_part;
    completed_part.SetETag(etag);
    completed_part.SetPartNumber(++i);
    complete_upload.AddParts(completed_part);
  }

  Aws::S3::Model::CompleteMultipartUploadRequest complete_upload_request;
  complete_upload_request.SetBucket(aws_bucket.c_str());
  complete_upload_request.SetKey(aws_key.c_str());
  complete_upload_request.SetUploadId(upload_id.c_str());
  complete_upload_request.WithMultipartUpload(complete_upload);
  auto request_outcome = client.CompleteMultipartUpload(complete_upload_request);

  if (!request_outcome.IsSuccess()) {
    throw runtime_error(request_outcome.GetError().GetMessage().c_str());
  }
}

std::string create_upload_promise(
  std::shared_ptr<pqxx::connection> db,
  const string &name,
  uint64_t bytes,
  const function<string(const string &, const string &, const string &)> &fn
) {
  // insert
  pqxx::work w(*db);
  string uuid = w.exec("select uuid_generate_v4()")[0][0].as<string>();
  string user = w.exec("select current_account_id()")[0][0].as<string>();
  string aws_region = "us-west-2";
  string aws_bucket = "havenofcode";
  string aws_key = "/" + user + "/" + uuid;
  // calculate how many parts
  // will need to be uploaded. each
  // part must be at least 5mb
  // and last part can be any size
  long double part_size = 5242880;
  int num_parts = ceil(bytes / part_size);
  string upload_id("");
  if (num_parts > 1) {
    upload_id = fn(aws_region, aws_bucket, aws_key);
  }
  // track the aws file part upload
  stringstream ss;
  ss << "insert into file (aws_region, aws_bucket, aws_key, bytes, name, upload_id) values ("
     << w.quote(aws_region) << ","
     << w.quote(aws_bucket) << ","
     << w.quote(aws_key) << ","
     << w.esc(to_string(bytes)) << ","
     << w.quote(name) << ","
     << ((num_parts > 1) ? w.quote(upload_id) : "NULL")
     << ") returning id";
  string id = w.exec(ss)[0][0].as<string>();
  ss.str("");
  ss.clear();
  for (int i = 0; i < num_parts; ++i) {
    ss << "insert into file_part (bytes, file, aws_part_number) values ("
       << w.esc(i + 1 == num_parts ? to_string(bytes - i * part_size) : "5242880")
       << ", " << w.quote(id) << ", " << i + 1 << "); ";
    w.exec(ss);
    ss.str("");
    ss.clear();
  }
  w.commit();
  return id;
}

void cancel_upload_promise(
  std::shared_ptr<pqxx::connection> db,
  const std::string &id,
  const std::function<void(const string &, const string &, const string &, const string &)> &fn
) {
  pqxx::work w(*db);
  stringstream ss;
  ss << "select status, aws_region, aws_bucket, aws_key, upload_id from file where id = " << w.quote(id);
  auto file_row = w.exec(ss);
  ss.str("");
  ss.clear();
  if (file_row.size() == 0) {
    throw runtime_error("file nonexistent");
  }
  auto status = file_row[0][0].as<string>();
  if (status != "pending") {
    throw runtime_error("cannot cancel file with status of " + status);
  }
  ss << "select count(*) from file_part where file = " << w.quote(id);
  auto part_count = w.exec(ss)[0][0].as<int>();
  if (part_count > 1) {
    fn(
      file_row[0][1].as<string>(),
      file_row[0][2].as<string>(),
      file_row[0][3].as<string>(),
      file_row[0][4].as<string>()
    );
  }
  ss.str("");
  ss.clear();
  ss << "update file set status = 'canceled' where "
     << "id = " << w.quote(id);
  w.exec(ss);
  ss.str("");
  ss.clear();
  ss << "update file_part set pending = 'FALSE' where file = " << w.quote(id);
  w.exec(ss);
  w.commit();
}

void start_file_part_promise(
  shared_ptr<pqxx::connection> db,
  const std::string &id
) {
  try {
    pqxx::work w(*db);
    stringstream ss;
    ss << "insert into file_part_promise (id) values (" << w.quote(id) << ")";
    w.exec(ss);
    w.commit();
  } catch (const std::exception &e) {
    throw runtime_error("promise taken");
  }
}

void complete_file_part_promise(
  std::shared_ptr<pqxx::connection> db,
  const string &file_part_id,
  const std::string &file_path,
  const function<string(
    const string &,
    const string &,
    const string &,
    const string &,
    const int,
    const std::string &
  )> &fn
) {
  pqxx::work w(*db);
  stringstream ss;
  ss << "select file_part_promise.id, file_part.bytes, file_part.aws_part_number, "
     << "file.aws_region, file.aws_bucket, file.aws_key, file.upload_id "
     << "from file, file_part, file_part_promise "
     << "where file_part_promise.id = file_part.id and "
     << "file_part.file = file.id and "
     << "file_part.id = " << w.quote(file_part_id);

  auto file_part = w.exec(ss);
  if (file_part.size() != 1) {
    throw runtime_error("promise not started");
  }

  auto bytes = file_part[0][1].as<string>();
  auto part_number = file_part[0][2].as<int>();
  auto aws_region = file_part[0][3].as<string>();
  auto aws_bucket = file_part[0][4].as<string>();
  auto aws_key = file_part[0][5].as<string>();
  auto aws_upload_id = file_part[0][6].is_null() ? "" : file_part[0][6].as<string>();
  string etag;

  try {
    etag = fn(aws_region, aws_bucket, aws_key, aws_upload_id, part_number, file_path);
  } catch (const exception &e) {
    ss.str("");
    ss.clear();
    ss << "delete from file_part_promise where id = " << w.quote(file_part[0][0].as<string>());
    w.exec(ss);
    w.commit();
    throw e;
  }

  ss.str("");
  ss.clear();
  ss << "update file_part set "
     << "aws_etag = " << ((etag == "") ? "NULL" : w.quote(etag)) << ", "
     << "pending = 'FALSE' where id = " << w.quote(file_part_id);
  w.exec(ss);
  w.commit();
}

void complete_upload_promise(
  std::shared_ptr<pqxx::connection> db,
  const string &id,
  const function<void(
    const string &,
    const string &,
    const string &,
    const string &,
    const vector<string> &
  )> &fn
) {
  pqxx::work w(*db);
  stringstream ss;
  ss << "select status, aws_region, aws_bucket, aws_key, upload_id from file where id = " << w.quote(id);
  auto file = w.exec(ss);
  if (file[0][0].as<string>() != "pending") {
    throw runtime_error("file status is " + file[0][0].as<string>());
  }
  ss.str("");
  ss.clear();
  ss << "select pending, aws_etag from file_part "
     << "where file = " << w.quote(id) << " order by aws_part_number asc";
  auto parts = w.exec(ss);
  vector<string> keys;
  if (parts.size() > 1) {
    for (size_t i = 0; i < parts.size(); ++i) {
      if (parts[i][0].as<bool>()) {
        // file part is still pending
        // cannot continue
        throw runtime_error("upload all file parts before calling complete");
      }
      keys.emplace_back(parts[i][1].as<string>());
    }
    fn(
      file[0][1].as<string>(),
      file[0][2].as<string>(),
      file[0][3].as<string>(),
      file[0][4].is_null() ? "" : file[0][4].as<string>(),
      keys
    );
  }
  ss.str("");
  ss.clear();
  ss << "update file set status = 'complete' where id = " << w.quote(id);
  w.exec(ss);
  w.commit();
}

} // actions
} // hoc
