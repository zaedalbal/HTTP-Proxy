// в этом файле идут объявление по типу "extern something"
#pragma once

#include "config/proxy_config.hpp"
#include "logger/logger.hpp"

namespace __PROXY_GLOBALS__
{
    extern Proxy_Config::Proxy_Settings PROXY_CONFIG;
    extern std::unordered_set<std::string> BLACKLISTED_HOSTS;
    extern bool LOG_ON;
    extern Logger LOGGER;
    extern Logger DEBUG_LOGGER;
    extern size_t ACTIVE_CONNECTIONS;
}