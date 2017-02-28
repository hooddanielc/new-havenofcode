#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <hoc/app.h>
#include <hoc/req.h>
#include <hoc/route.h>

#include <hoc/routes/login.h>

namespace hoc {
  void assign_routes();
  void route_request(const req_t &req);
}
