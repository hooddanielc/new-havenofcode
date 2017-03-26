#include <hoc/actions/account.h>

using namespace std;
using namespace hoc;

namespace hoc {
namespace actions {

void confirm_account(const std::string &email, const std::string &password) {
  auto c = db::admin_connection();
  pqxx::work w(*c);
  stringstream ss;
  ss << "select salt from registration where email = " << w.quote(email);
  pqxx::result r = w.exec(ss);
  auto salt = r[0][0].as<string>();

  ss.str(string());
  ss.clear();

  ss << "select password from registration where "
     << "email = " << w.quote(email) << " AND "
     << "password = md5(" << w.quote(password + salt) << ")";
  r = w.exec(ss);
  auto hash = r[0][0].as<string>();

  if (r.size() == 1) {
    ss.str(string());
    ss.clear();
    ss << "create user " << w.quote_name(email) << " with password " << w.quote(hash);
    w.exec(ss);
    ss.str(string());
    ss.clear();
    ss << "insert into account (email, salt) "
       << "values ("
       << w.quote(email) << ","
       << w.quote(salt)
       << ")";
    w.exec(ss);
    w.commit();
  } else {
    throw std::runtime_error("incorrect password or email");
  }
}

void register_account(const std::string &email, const std::string &password) {
  auto c = db::anonymous_connection();
  std::string salt(random_characters(32));
  pqxx::work w(*c);
  stringstream ss;

  ss << "insert into registration (email, salt, password) "
     << "values ("
     << w.quote(email) << ","
     << w.quote(salt) << ","
     << "md5(" << w.quote(password + salt) << ")"
     << ")";

  pqxx::result r = w.exec(ss);
  w.commit();
}

void login(const std::string &email, const std::string &password) {
  
}

}
}