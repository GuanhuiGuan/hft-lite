#pragma once

#include <string>
#include <print>
#include <netdb.h>
#include <sys/types.h>
#include <ifaddrs.h>

namespace common {

// get interface ip
std::string get_iface_ip(const std::string& name);

} // namespace common