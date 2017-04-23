#include <hoc/actions/file.h>

using namespace std;

namespace hoc {
namespace actions {

string create_aws_multipart_upload(
  const string &aws_region,
  const string &aws_bucket,
  const string &aws_key
) {
  return "todo";
}

void cancel_aws_multipart_upload(
  const string &aws_region,
  const string &aws_bucket,
  const string &aws_key,
  const string &upload_id
) {
  // todo
}

std::string complete_aws_file_part_promise(
  const string &aws_region,
  const string &aws_bucket,
  const string &aws_key,
  const string &upload_id,
  const int part_number,
  const vector<uint8_t> &
) {
  return "todo";
}

void complete_aws_multipart_upload(
  const string &aws_region,
  const string &aws_bucket,
  const string &aws_key,
  const string &upload_id,
  const vector<string> &keys
) {
  // todo
}

std::string create_upload_promise(
  std::shared_ptr<pqxx::connection> db,
  const string &name,
  int64_t bits,
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
  long double part_size = 40000000;
  int num_parts = ceil(bits / part_size);
  string upload_id("");
  if (num_parts > 1) {
    upload_id = fn(aws_region, aws_bucket, aws_key);
  }
  // track the aws file part upload
  stringstream ss;
  ss << "insert into file (aws_region, aws_bucket, aws_key, bits, name, upload_id) values ("
     << w.quote(aws_region) << ","
     << w.quote(aws_bucket) << ","
     << w.quote(aws_key) << ","
     << w.esc(to_string(bits)) << ","
     << w.quote(name) << ","
     << ((num_parts > 1) ? w.quote(upload_id) : "NULL")
     << ") returning id";
  string id = w.exec(ss)[0][0].as<string>();
  ss.str("");
  ss.clear();
  for (int i = 0; i < num_parts; ++i) {
    ss << "insert into file_part (bits, file, aws_part_number) values ("
       << w.esc(i + 1 == num_parts ? to_string(bits - i * part_size) : "40000000")
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
  const vector<uint8_t> &data,
  const function<string(
    const string &,
    const string &,
    const string &,
    const string &,
    const int,
    const vector<uint8_t> &
  )> &fn
) {
  pqxx::work w(*db);
  stringstream ss;
  ss << "select file_part_promise.id, file_part.bits, file_part.aws_part_number, "
     << "file.aws_region, file.aws_bucket, file.aws_key, file.upload_id "
     << "from file, file_part, file_part_promise "
     << "where file_part_promise.id = file_part.id and "
     << "file_part.file = file.id and "
     << "file_part.id = " << w.quote(file_part_id);

  auto file_part = w.exec(ss);
  if (file_part.size() != 1) {
    throw runtime_error("promise not started");
  }

  auto bits = file_part[0][1].as<string>();
  auto part_number = file_part[0][2].as<int>();
  auto aws_region = file_part[0][3].as<string>();
  auto aws_bucket = file_part[0][4].as<string>();
  auto aws_key = file_part[0][5].as<string>();
  auto aws_upload_id = file_part[0][6].is_null() ? "" : file_part[0][6].as<string>();
  auto etag = fn(aws_region, aws_bucket, aws_key, aws_upload_id, part_number, data);

  ss.str("");
  ss.clear();
  ss << "update file_part set "
     << "aws_etag = " << ((etag == "") ? "NULL" : w.quote(etag)) << ", "
     << "pending = 'FALSE'";
  w.exec(ss);
  w.commit();
}

void complete_upload_promise(
  std::shared_ptr<pqxx::connection> db,
  const string &id,
  const function<string(
    const string &,
    const string &,
    const string &,
    const string &,
    const vector<string> &
  )> &fn
) {
  // todo
}

} // actions
} // hoc
