#include <hoc/routes/all.h>

using namespace std;

namespace hoc {
  vector<unique_ptr<route_t<req_t>>> routes;

  void assign_routes()  {
    routes.push_back(unique_ptr<route_t<req_t>>(new echo_route_t<req_t>()));
    routes.push_back(unique_ptr<route_t<req_t>>(new login_route_t<req_t>()));
    routes.push_back(unique_ptr<route_t<req_t>>(new register_route_t<req_t>()));
    routes.push_back(unique_ptr<route_t<req_t>>(new set_noreply_token_route_t<req_t>()));
    routes.push_back(unique_ptr<route_t<req_t>>(new set_noreply_token_callback_route_t<req_t>()));
    routes.push_back(unique_ptr<route_t<req_t>>(new confirm_registration_route_t<req_t>()));
  }

  void end_server_error(req_t &req, const string &message) {
    req.set_status(500);
    auto json = dj::json_t::empty_object;
    json["error"] = true;
    json["message"] = message;
    string out(json.to_string());
    req.set_content_length(out.size());
    req.send_body(out);
  }

  void route_request(req_t &req) {
    try {
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
    } catch (runtime_error e) {
      end_server_error(req, e.what());
    } catch (logic_error e) {
      end_server_error(req, e.what());
    } catch (exception e) {
      end_server_error(req, e.what());
    } catch (...) {
      end_server_error(req, "something terrible happened");
    }
  }
}
