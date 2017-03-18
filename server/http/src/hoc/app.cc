#include <hoc/app.h>

using namespace std;

namespace hoc {
  void app_t::on_request(const cb_request_t &fn) {
    request_events.push_back(fn);
  }

  void app_t::on_start(const cb_void_t &fn) {
    start_events.push_back(fn);
  }

  void app_t::on_exit(const cb_void_t &fn) {
    exit_events.push_back(fn);
  }

  cb_request_list_t app_t::get_request_events() {
    return request_events;
  }

  cb_void_list_t app_t::get_start_events() {
    return start_events;
  }

  cb_void_list_t app_t::get_exit_events() {
    return exit_events;
  }

  app_t &app_t::clear() {
    request_events.clear();
    start_events.clear();
    exit_events.clear();
    return *this;
  }

  void app_t::emit_request(req_t &request) {
    for (auto it = request_events.begin(); it < request_events.end(); ++it) {
      (*it)(request);
    }
  }

  template<typename T>
  void app_t::emit_void(T &list) {
    for (auto it = list.begin(); it < list.end(); ++it) {
      (*it)();
    }
  }

  void app_t::emit_start() {
    emit_void<cb_void_list_t>(start_events);
  }

  void app_t::emit_exit() {
    emit_void<cb_void_list_t>(exit_events);
  }

  void app_t::log(const std::string &out) {
    ofstream log_file("/root/logs/stdout.log", ios_base::out | ios_base::app );
    log_file << out << endl;
  }
}
