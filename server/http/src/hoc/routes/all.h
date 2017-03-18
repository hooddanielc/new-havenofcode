#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <hoc/app.h>
#include <hoc/req.h>
#include <hoc/route.h>

#include <hoc/routes/login.h>
#include <hoc/routes/register.h>
#include <hoc/routes/set_noreply_token.h>
#include <hoc/routes/set_noreply_token_callback.h>
#include <hoc/routes/echo.h>
#include <hoc/routes/confirm_registration.h>

namespace hoc {
  void assign_routes();
  void route_request(req_t &req);
}
