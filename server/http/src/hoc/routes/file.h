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
#include <hoc/parsers/multipart.h>

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
      ss.str("");
      ss.clear();
      ss << "select aws_etag, aws_part_number, bytes, created_at, "
         << "created_by, id, pending, updated_at, file from file_part "
         << "where pending = 'f' and created_by = current_account_id()";
      auto file_parts_result = w.exec(ss);
      auto files = to_json(result);
      auto file_parts = to_json(file_parts_result);
      auto json = dj::json_t::empty_object;
      json["files"] = std::move(files);
      json["fileParts"] = std::move(file_parts);
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

  void put(T &req, const url_match_result_t &match, std::shared_ptr<session_t<req_t>> &session) {
    if (!session->authenticated()) {
      return route_t<T>::fail_with_error(req, "login required", 403);
    }

    auto file_part_id = std::shared_ptr<std::string>(new std::string(match.params[0]));
    actions::start_file_part_promise(session->db, *file_part_id);

    const std::string search("multipart/form-data; boundary=");
    size_t n = req.request_headers["Content-Type"][0].find(search);
    if (n == std::string::npos) {
      route_t<T>::fail_with_error(req, "only multipart/form-data requests are supported");
    }

    auto boundary = req.request_headers["Content-Type"][0].substr(search.size());
    auto parser = std::shared_ptr<multipart_binary_parser_t>(
      new multipart_binary_parser_t(boundary.c_str())
    );

    auto should_capture = std::make_shared<bool>(false);
    parser->on_header([should_capture](const std::map<std::vector<uint8_t>, std::vector<uint8_t>> &headers) {
      std::map<std::string, std::string> content;
      const char *type = "Content-Type";
      const char *disp = "Content-Disposition";
      std::vector<uint8_t> v_type(type, type + strlen(type));
      std::vector<uint8_t> v_disp(disp, disp + strlen(disp));

      if (!headers.count(v_type) || !headers.count(v_disp)) {
        return;
      }

      auto t_data = headers.find(v_type)->second;
      auto d_data = headers.find(v_disp)->second;
      std::string type_val(t_data.data(), t_data.data() + t_data.size());
      std::string disp_val(d_data.data(), d_data.data() + d_data.size());
      if (
        type_val == "application/octet-stream" &&
        disp_val == "form-data; name=\"blob\"; filename=\"blob\""
      ) {
        (*should_capture) = true;
      } else {
        (*should_capture) = false;
      }
    });

    auto path = std::shared_ptr<std::string>(new std::string(random_tmp_path()));
    auto out = std::shared_ptr<std::ofstream>(new std::ofstream(*path));

    parser->on_body([should_capture, out](const std::vector<uint8_t> &body) {
      if ((*should_capture)) {
        std::copy(body.begin(), body.end(), std::ostreambuf_iterator<char>(*out));
      }
    });

    parser->on_multipart_end([out]() {
      out->close();
    });

    req.on_data([out, parser](const std::vector<uint8_t> &data) {
      data >> (*parser);
    });

    req.on_end([&, path, file_part_id, session]() {
      actions::complete_file_part_promise(session->db, *file_part_id, *path);
      pqxx::work w(*session->db);
      std::stringstream ss;
      ss << "select aws_etag, aws_part_number, bytes, created_at, "
         << "created_by, id, pending, updated_at, file from file_part "
         << "where id = " << w.quote(*file_part_id);
      auto res = w.exec(ss);
      auto json = dj::json_t::empty_object;
      json["filePart"] = to_json(res)[0];
      unlink(path->c_str());
      route_t<T>::send_json(req, json, 200);
    });
  }
};

} // hoc
