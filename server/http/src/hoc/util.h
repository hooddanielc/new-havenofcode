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
#include <hoc/json.h>

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
  char *random_characters(size_t size);
  std::string camelify(std::string &str);
  dj::json_t to_json(const pqxx::result &result);
}

