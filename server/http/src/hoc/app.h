#pragma once
#include <iostream>
#include <map>
#include <vector>
#include <functional>
#include <string>
#include <cstdio>
#include <fstream>
#include <hoc/req.h>

namespace hoc {

using cb_request_t = std::function<void(const req_t &request)>;
using cb_void_t = std::function<void()>;

using cb_request_list_t = std::vector<cb_request_t>;
using cb_void_list_t = std::vector<cb_void_t>;

class app_t final {
  public:
    static app_t &get() {
      static app_t singleton;
      return singleton;
    }

    void on_request(const cb_request_t &fn);
    void on_start(const cb_void_t &fn);
    void on_exit(const cb_void_t &fn);

    void emit_request(const req_t &request);
    void emit_start();
    void emit_exit();
    void log(const std::string &str);

    // defined in <hoc/main.cc>
    void main();

    cb_request_list_t get_request_events();
    cb_void_list_t get_start_events();
    cb_void_list_t get_exit_events();
    int status;

    app_t &clear();

  private:
    cb_request_list_t request_events;
    cb_void_list_t start_events;
    cb_void_list_t exit_events;

    template<typename T>
    void emit_void(const T &list);

    app_t(): status(200) {};
    app_t(app_t &&) = delete;
    app_t(const app_t &) = delete;
    ~app_t() = default;
    app_t &operator=(app_t &&) = delete;
    app_t &operator=(const app_t &) = delete;

}; // app_t

}; // hoc