#pragma once

#include <vector>
#include <string>
#include <functional>
#include <map>

namespace hoc {

class req_t final {
public:

  using cb_data_t = std::function<void(const std::string &data)>;
  using cb_data_list_t = std::vector<cb_data_t>;
  using header_list_t = std::map<std::string, std::string>;

  req_t(const header_list_t &headers): headers(headers) {};
  void on_data(const cb_data_t &fn) const;
  void emit_data(const std::string &data);
  header_list_t headers;

private:

  mutable cb_data_list_t data_events;


};  // req_t

}  // hoc
