#pragma once

#include <cstdlib>
#include <cstdint>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include <hoc/util.h>
#include <hoc/route.h>
#include <hoc/actions/file.h>

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
      auto files = to_json(result);
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

      char *endptr;
      auto id = actions::create_upload_promise(session->db, name, strtoimax(bytes.c_str(), &endptr, 10));
      pqxx::work w(*session->db);
      std::stringstream ss;
      ss << "select id, created_by, created_at, updated_at, name, "
         << "aws_key, aws_region, bytes, status, progress, upload_id "
         << "from file where id = " << w.quote(id);
      auto file_query = w.exec(ss);

      // select all the file_parts
      ss.str("");
      ss.clear();
      ss << "select id, created_at, updated_at, bytes, aws_etag, "
         << "aws_part_number, pending, created_by "
         << "from file_part where file = " << w.quote(id) << " order by aws_part_number asc";
      auto file_part_query = w.exec(ss);
      auto files = to_json(file_query);
      auto file_part_ids = to_json_array<std::string>(file_part_query, 0);
      auto file_parts = to_json(file_part_query);
      auto response = dj::json_t::empty_object;
      response["files"] = files;
      response["files"][0]["fileParts"] = file_part_ids;
      response["fileParts"] = file_parts;
      route_t<T>::send_json(req, response, 200);
    });
  }
};

template<typename T>
class file_part_route_t : public route_t<T> {
public:
  file_part_route_t() : route_t<T>("/api/file-part/:id") {}

  void put(T &req, const url_match_result_t &, std::shared_ptr<session_t<req_t>> &session) {
    if (!session->authenticated()) {
      return route_t<T>::fail_with_error(req, "login required", 403);
    }

    auto path = std::shared_ptr<std::string>(new std::string(random_tmp_path()));
    int fd = creat(path->c_str(), 0644);

    req.on_data([fd](const std::vector<uint8_t> &data) {
      write(fd, &data[0], data.size());
    });

    req.on_end([&, fd, path]() {
      close(fd);

      // delete the file
      unlink(path->c_str());
      route_t<T>::fail_with_error(req, "not implemented", 500);
    });
  }
};

} // hoc
