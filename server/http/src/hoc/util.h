#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <istream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <stdlib.h>
#include <ctype.h>
#include <stdexcept>
#include <pqxx/pqxx>
#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/S3Client.h>
#include <hoc/json.h>
#include <hoc/env.h>

namespace hoc {
  std::string url_encode(const std::string &str);
  std::string url_decode(const std::string &str);

  template<typename out_t>
  void split(const std::string &s, char delim, out_t &result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
      result.push_back(item);
    }
  }

  void open_ifstream(std::ifstream &strm, const std::string &path);
  std::string random_characters(size_t size);
  std::string camelify(std::string &str);
  dj::json_t::array_t to_json(const pqxx::result &result);

  template <typename as_t>
  dj::json_t::array_t to_json_array(const pqxx::result &rows, int i) {
    dj::json_t::array_t result;
    for (auto it = rows.begin(); it != rows.end(); ++it) {
      result.emplace_back(it[i].as<as_t>());
    }
    return result;
  }

  template <typename as_t>
  dj::json_t::array_t to_json_array(const pqxx::result &rows, const char *name) {
    dj::json_t::array_t result;
    auto num = rows.column_number(name);
    for (auto it = rows.begin(); it != rows.end(); ++it) {
      result.emplace_back(it[num].as<as_t>());
    }
    return result;
  }

  std::string random_tmp_path();
  std::string get_s3_url(const dj::json_t &file);
}
