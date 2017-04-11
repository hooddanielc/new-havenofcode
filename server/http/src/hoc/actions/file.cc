#include <hoc/actions/file.h>

using namespace std;

namespace hoc {
namespace actions {

string create_aws_multipart_upload() {
  return "todo";
}

void cancel_aws_multipart_upload() {
  // todo
}

void complete_aws_multipart_upload() {
  // todo
}

std::string create_upload_promise(
  std::shared_ptr<pqxx::connection> db,
  const string &name,
  int64_t bits,
  const function<string()> &fn
) {
  // calculate how many parts 
  // will need to be uploaded. each
  // part must be at least 5mb
  long double part_size = 40000000;
  int num_parts = ceil(bits / part_size);
  string upload_id("");
  if (num_parts > 1) {
    upload_id = fn();
  }

  // insert
  pqxx::work w(*db);
  string uuid = w.exec("select uuid_generate_v4()")[0][0].as<string>();
  string user = w.exec("select current_account_id()")[0][0].as<string>();
  stringstream ss;
  ss << "insert into file (aws_key, bits, name, upload_id) values ("
     << w.quote("/" + user + "/" + uuid) << ","
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
  const std::function<void()> &fn
) {
  pqxx::work w(*db);
  stringstream ss;
  ss << "select status, bits from file where id = " << w.quote(id);
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
    fn();
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

void complete_upload_promise(
  std::shared_ptr<pqxx::connection> db,
  const std::string &id,
  const std::function<void()> &fn
) {
  // todo
}

} // actions
} // hoc
