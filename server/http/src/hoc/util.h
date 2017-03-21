#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <stdlib.h>
#include <ctype.h>

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
}

