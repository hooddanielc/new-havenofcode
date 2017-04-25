#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <hoc/util.h>

extern "C" {
  #include <ngx_config.h>
  #include <ngx_core.h>
  #include <ngx_http.h>
}

namespace hoc {

class req_t final {
  public:

    using cb_data_t = std::function<void(const std::vector<uint8_t> &data)>;
    using cb_file_t = std::function<void(const std::string &file)>;
    using cb_void_t = std::function<void()>;
    using cb_data_list_t = std::vector<cb_data_t>;
    using cb_file_list_t = std::vector<cb_file_t>;
    using cb_void_list_t = std::vector<cb_void_t>;
    using header_list_t = std::map<std::string, std::vector<std::string>>;

    ngx_http_request_t *request_context;
    ngx_chain_t *out;

    req_t(
      const header_list_t &_request_headers
    ) :
        request_context(nullptr),
        out(nullptr),
        request_headers(_request_headers) {}

    req_t(
      const header_list_t &_request_headers,
      ngx_http_request_t *_request_context
    ) : request_context(_request_context),
        out((ngx_chain_t*) ngx_pcalloc(_request_context->pool, sizeof(ngx_chain_t))),
        request_headers(_request_headers) {}

    void on_data(const cb_data_t &fn);
    void on_end(const cb_void_t &fn);
    void on_file(const cb_file_t &fn);
    void emit_data(const std::vector<uint8_t> &data);
    void emit_end();
    void emit_file(const std::string &path);
    void set_content_length(int len);
    void send_header(const std::string &key, const std::string &val);
    void send_body(const std::string &data);
    void set_status(int status);
    std::string uri();
    std::string method();
    std::string args();
    std::string request_line();
    std::string exten();
    std::string unparsed_uri();
    header_list_t request_headers;
    std::map<std::string, std::string> query();
    std::map<std::string, std::vector<std::string>> cookies();
    std::string ip();
    std::string user_agent();

    // delete copy
    req_t(const req_t &) = delete;
    req_t &operator=(const req_t &) = delete;

  private:
    cb_data_list_t data_events;
    cb_file_list_t file_events;
    cb_void_list_t end_events;
};  // req_t

}  // hoc
