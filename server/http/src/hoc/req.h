#pragma once

#include <vector>
#include <string>
#include <functional>
#include <map>

namespace hoc {

class req_t final {
public:

  using cb_data_t = std::function<void(const std::string &data)>;
  using cb_void_t = std::function<void()>;
  using cb_data_list_t = std::vector<cb_data_t>;
  using cb_void_list_t = std::vector<cb_void_t>;
  using header_list_t = std::map<std::string, std::string>;

  req_t(
    const header_list_t &request_headers
  ): request_headers(request_headers) {};

  void on_data(const cb_data_t &fn) const;
  void on_end(const cb_void_t &fn) const;
  void emit_data(const std::string &data);
  void emit_end();
  void set_content_length(int len) const;
  void send_header(const std::string &key, const std::string &val) const;
  void send_body(const std::string &data) const;
  void set_status(int status) const;
  std::string uri() const;
  std::string method() const;
  std::string args() const;
  std::string request_line() const;
  std::string exten() const;
  std::string unparsed_uri() const;
  header_list_t request_headers;

private:

  mutable cb_data_list_t data_events;
  mutable cb_void_list_t end_events;


};  // req_t

}  // hoc
