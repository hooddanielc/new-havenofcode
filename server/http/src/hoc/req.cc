#include <hoc/req.h>

using namespace std;

namespace hoc {

void req_t::on_data(const req_t::cb_data_t &fn) const {
  data_events.push_back(fn);
}

void req_t::emit_data(const string &data) {
  for (auto it = data_events.begin(); it != data_events.end(); ++it) {
    (*it)(data);
  }
}

} // hoc