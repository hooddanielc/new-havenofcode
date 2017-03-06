#include <hoc/routes/all.h>

using namespace std;

namespace hoc {
  vector<unique_ptr<route_t<req_t>>> routes;

  void assign_routes()  {
    routes.push_back(unique_ptr<route_t<req_t>>(new login_route_t<req_t>()));
    routes.push_back(unique_ptr<route_t<req_t>>(new register_route_t<req_t>()));
    routes.push_back(unique_ptr<route_t<req_t>>(new set_noreply_token_route_t<req_t>()));
    routes.push_back(unique_ptr<route_t<req_t>>(new set_noreply_token_callback_route_t<req_t>()));
  }

  void route_request(const req_t &req) {
    for (auto &route : routes) {
      auto match = route->match(req.uri().c_str());

      if (match.pass == true) {
        if (req.method() == "GET") {
          route->get(req, match);
        } else if (req.method() == "POST") {
          route->post(req, match);
        } else if (req.method() == "PUT") {
          route->put(req, match);
        } else if (req.method() == "DELETE") {
          route->del(req, match);
        }
      }
    }
  }
}
