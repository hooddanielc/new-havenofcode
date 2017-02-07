#include <hoc/req.h>

using namespace std;

namespace hoc {

void req_t::on_data(const req_t::cb_data_t &fn) const {
  data_events.push_back(fn);
}

void req_t::on_end(const req_t::cb_void_t &fn) const {
  end_events.push_back(fn);
} 

void req_t::emit_data(const string &data) {
  for (auto it = data_events.begin(); it != data_events.end(); ++it) {
    (*it)(data);
  }
}

void req_t::emit_end() {
  for (auto it = end_events.begin(); it != end_events.end(); ++it) {
    (*it)();
  }
}

} // hoc