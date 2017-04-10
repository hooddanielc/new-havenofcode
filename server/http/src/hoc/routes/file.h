#pragma once

#include <cstdlib>
#include <hoc/route.h>

namespace hoc {

template<typename T>
class file_route_query_t : public route_t<T> {
public:
  file_route_query_t() : route_t<T>("/api/files") {}

  void get(T &req, const url_match_result_t &, std::shared_ptr<session_t<req_t>> &session) {
    try {
      if (!session->authenticated()) {
        return route_t<T>::fail_with_error(req, "login required", 403);
      }

      auto args = req.query();
      std::string offset("0");
      std::string limit("30");
      std::string created_by("");

      if (args.count("offset")) {
        offset = args["offset"];
      }

      if (args.count("limit")) {
        limit = args["limit"];
      }

      if (atoi(limit.c_str()) > 100) {
        limit = "100";
      }

      if (args.count("created_by")) {
        if (args["created_by"] == "me") {
          created_by = session->user_id();
        } else {
          created_by = args["created_by"];
        }
      }

      pqxx::work w(*session->db);
      std::stringstream ss;
      ss << "select id, created_at, updated_at, created_by, aws_key, "
         << "aws_region, bits, status, progress from file where id is not null ";

      if (created_by != "") {
        ss << "and created_by = " << w.quote(created_by) << " ";
      }

      ss << "limit " << w.esc(limit) << " offset " << w.esc(offset);
      auto result = w.exec(ss);
      dj::json_t::array_t users;

      for (size_t i = 0; i < result.size(); ++i) {
        auto user = dj::json_t::empty_object;
        user["id"] = result[i][0].as<std::string>();
        user["created_at"] = result[i][1].as<std::string>();
        user["updated_at"] = result[i][2].as<std::string>();
        user["created_by"] = result[i][3].as<std::string>();
        user["aws_key"] = result[i][4].as<std::string>();
        user["aws_region"] = result[i][5].as<std::string>();
        user["bits"] = result[i][6].as<std::string>();
        user["status"] = result[i][7].as<std::string>();
        user["progress"] = result[i][8].as<std::string>();
        users.emplace_back(std::move(user));
      }

      auto json = dj::json_t::empty_object;
      json["users"] = std::move(users);
      route_t<T>::send_json(req, json, 200);
    } catch (const std::exception &e) {
      route_t<T>::fail_with_error(req, e.what());
    }
  }
};

} // hoc
