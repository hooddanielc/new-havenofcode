#include <hoc/actions/account.h>

using namespace std;
using namespace hoc;

namespace hoc {
namespace actions {

pqxx::result confirm_account(const std::string &email, const std::string &password) {
  auto c = db::super_user_connection();
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

  if (r.size() == 0) {
    throw std::runtime_error("incorrect password or email");
  }

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
     << ") returning id, email";

  auto r_insert_account = w.exec(ss);

  ss.str(string());
  ss.clear();
  ss << "grant members to " << w.quote_name(email);
  w.exec(ss);

  ss.str(string());
  ss.clear();
  ss << "set role " << w.quote_name(email);
  w.exec(ss);

  ss.str(string());
  ss.clear();
  ss << "set session authorization " << w.quote_name(email);
  w.exec(ss);

  ss.str(string());
  ss.clear();
  ss << "update registration set verified = 'TRUE' where email = " << w.quote(email);
  w.exec(ss);

  w.commit();
  return r_insert_account;
}

pqxx::result register_account(const std::string &email, const std::string &password) {
  auto c = db::anonymous_connection();
  string salt(random_characters(32));
  pqxx::work w(*c);
  stringstream ss;

  ss << "select id from registration where email = " << w.quote(email);
  auto r = w.exec(ss);
  ss.str(string());
  ss.clear();

  if (r.size() == 0) {
    ss << "insert into registration (email, salt, password) "
       << "values ("
       << w.quote(email) << ","
       << w.quote(salt) << ","
       << "md5(" << w.quote(password + salt) << ")"
       << ") returning id, password";
  } else {
    ss << "update registration set "
       << "salt = " << w.quote(salt) << ","
       << "password = md5(" << w.quote(password + salt) << ") "
       << "where email = " << w.quote(email) << " "
       << "returning id, password";
  }

  auto result = w.exec(ss);
  w.commit();
  return result;
}

pqxx::result login(
  const string &email,
  const string &password,
  const string &ip,
  const string &user_agent
) {
  auto c = db::member_connection(email, password);
  pqxx::work w(*c);
  stringstream ss;
  ss << "insert into session (ip, user_agent, created_by) VALUES ("
     << w.quote(ip) << ","
     << w.quote(user_agent) << ","
     << "current_account_id()"
     << ") returning id, created_by";
  auto r_session = w.exec(ss);
  w.commit();
  return r_session;
}

pqxx::result restore_session(
  const string &uuid,
  const string &ip,
  const string &user_agent
) {
  auto c = db::session_connection(uuid);
  pqxx::work w(*c);
  stringstream ss;

  ss << "select user_agent, ip from session where "
     << "created_by = current_account_id() and "
     << "id = " << w.quote(uuid);

  auto r_session = w.exec(ss);

  if (r_session.size() == 0) {
    throw runtime_error("invalid session");
  }

  if (r_session[0][0].as<string>() != user_agent) {
    ss.str(string());
    ss.clear();
    ss << "update session set deleted = 'TRUE' where id = " << w.quote(uuid);
    w.exec(ss);
    w.commit();
    throw runtime_error("invalid session");
  }

  ss.str(string());
  ss.clear();
  ss << "update session set updated_at = 'NOW()' where id = " << w.quote(uuid);
  w.exec(ss);

  ss.str(string());
  ss.clear();
  ss << "insert into session_ip_log (ip, session, created_by) values ("
     << w.quote(ip) << ","
     << w.quote(uuid) << ","
     << "current_account_id()"
     << ")";
  w.exec(ss);

  w.commit();
  return r_session;
}

}
}