#pragma once

#include <sstream>
#include <pqxx/pqxx>
#include <hoc/env.h>
#include <hoc/db/connection.h>
#include <hoc/util.h>

namespace hoc {
namespace actions {

void confirm_account(const std::string &email, const std::string &password);
void register_account(const std::string &email, const std::string &password);
void login(const std::string &email, const std::string &password);

}
}