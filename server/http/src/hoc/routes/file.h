#pragma once

#include <cstdlib>
#include <cstdint>
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
         << "aws_region, bytes, status, progress from file where id is not null ";

      if (created_by != "") {
        ss << "and created_by = " << w.quote(created_by) << " ";
      }

      ss << "limit " << w.esc(limit) << " offset " << w.esc(offset);
      auto result = w.exec(ss);
      dj::json_t::array_t files;

      for (size_t i = 0; i < result.size(); ++i) {
        auto user = dj::json_t::empty_object;
        user["id"] = result[i][0].as<std::string>();
        user["createdAt"] = result[i][1].as<std::string>();
        user["updatedAt"] = result[i][2].as<std::string>();
        user["createdBy"] = result[i][3].as<std::string>();
        user["awsKey"] = result[i][4].as<std::string>();
        user["awsRegion"] = result[i][5].as<std::string>();
        user["bytes"] = result[i][6].as<std::string>();
        user["status"] = result[i][7].as<std::string>();
        user["progress"] = result[i][8].as<std::string>();
        files.emplace_back(std::move(user));
      }

      auto json = dj::json_t::empty_object;
      json["files"] = std::move(files);
      route_t<T>::send_json(req, json, 200);
    } catch (const std::exception &e) {
      route_t<T>::fail_with_error(req, e.what());
    }
  }

  void post(T &req, const url_match_result_t &, std::shared_ptr<session_t<req_t>> &session) {
    if (!session->authenticated()) {
      return route_t<T>::fail_with_error(req, "login required", 403);
    }

    auto str = new std::string();
    req.on_data([str](const std::vector<uint8_t> &data) {
      str->append(data.begin(), data.end());
    });

    req.on_end([&, str, session]() {
      dj::json_t json;

      try {
        json = dj::json_t::from_string((*str).c_str());
        delete str;
      } catch (const std::exception &e) {
        delete str;
        return route_t<T>::fail_with_error(req, "invalid json");
      }

      if (!json.contains("file")) {
        return route_t<T>::fail_with_error(req, "file object required");
      }

      std::string bytes("");
      if (!json["file"].contains("bytes")) {
        return route_t<T>::fail_with_error(req, "bytes property required in file object");
      } else {
        bytes = json["file"]["bytes"].as<std::string>();
      }

      std::string name("unnamed");
      if (json["file"].contains("name")) {
        name = json["file"]["name"].as<std::string>();
      }

      pqxx::work w(*session->db);
      std::stringstream ss;
      ss << "insert into file (bytes, name) values ("
         << w.esc(bytes) << ","
         << w.quote(name) << ") returning "
         << "id, created_by, created_at, updated_at, name, aws_key, aws_region, "
         << "bytes, status, progress";

      try {
        auto res = w.exec(ss);
        w.commit();
        auto json = dj::json_t::empty_object;
        json["files"] = dj::json_t::empty_object;
        json["files"]["id"] = res[0][0].as<std::string>();
        json["files"]["createdBy"] = res[0][1].as<std::string>();
        json["files"]["createdAt"] = res[0][2].as<std::string>();
        json["files"]["updatedAt"] = res[0][3].as<std::string>();
        json["files"]["name"] = res[0][4].as<std::string>();
        json["files"]["awsKey"] = res[0][5].as<std::string>();
        json["files"]["awsRegion"] = res[0][6].as<std::string>();
        json["files"]["bytes"] = res[0][7].as<std::string>();
        json["files"]["status"] = res[0][8].as<std::string>();
        json["files"]["progress"] = res[0][9].as<std::string>();
        route_t<T>::send_json(req, json, 200);
      } catch (const std::exception &e) {
        route_t<T>::fail_with_error(req, e.what());
      }
    });
  }
};

} // hoc
