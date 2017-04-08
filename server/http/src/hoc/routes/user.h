#pragma once

#include <string>
#include <sstream>
#include <stdexcept>
#include <hoc/json.h>

namespace hoc {

template<typename T>
class user_route_single_t : public route_t<T> {
public:
  user_route_single_t() : route_t<T>("/api/users/:id") {}

  // this route allows only current user
  // to be selected
  void get(T &req, const url_match_result_t &match, std::shared_ptr<session_t<req_t>> &session) {
    if (session->authenticated() && match.params[0] == "me") {
      pqxx::work w(*session->db);
      std::stringstream ss;
      ss << "select email from account where id = "
         << w.quote(session->user_id());
      auto res = w.exec(ss);
      auto json = dj::json_t::empty_object;
      json["user"] = dj::json_t::empty_object;
      json["user"]["email"] = res[0][0].as<std::string>();
      json["user"]["id"] = session->user_id();
      route_t<T>::send_json(req, json, 200);
    } else {
      return route_t<T>::fail_with_error(req, "login required");
    }
  }
};

template<typename T>
class user_route_query_t : public route_t<T> {
public:
  user_route_query_t() : route_t<T>("/api/users") {}

  void get(T &req, const url_match_result_t &, std::shared_ptr<session_t<req_t>> &session) {
    try {
      auto args = req.query();

      if (args.count("email")) {
        pqxx::work w(*session->db);
        std::stringstream ss;
        ss << "select email, id from account where email = "
           << w.quote(args["email"]);
        auto res = w.exec(ss);
        if (res.size() == 0) {
          return route_t<T>::fail_with_error(req, "not found", 404);
        }

        auto json = dj::json_t::empty_object;
        json["user"] = dj::json_t::empty_object;
        json["user"]["email"] = res[0][0].as<std::string>();
        json["user"]["id"] = res[0][1].as<std::string>();
        route_t<T>::send_json(req, json, 200);
      } else {
        route_t<T>::fail_with_error(req, "email url param required");
      }
    } catch (const std::exception &e) {
      std::cout << e.what() << std::endl;
      route_t<T>::fail_with_error(req, e.what());
    } catch (const std::runtime_error &e) {
      std::cout << e.what() << std::endl;
      route_t<T>::fail_with_error(req, e.what());
    }
  }
};

} // hoc
