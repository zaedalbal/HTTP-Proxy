#pragma once
#include "proxy_config.hpp"
#include <unordered_set>

extern Proxy_Config::Proxy_Settings PROXY_CONFIG;
extern std::unordered_set<std::string> BLACKLISTED_HOSTS;