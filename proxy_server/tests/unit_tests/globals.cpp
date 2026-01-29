#include "logger/logger.hpp"
#include "config/proxy_config.hpp"

// определения глобальных переменных
Proxy_Config::Proxy_Settings PROXY_CONFIG;
std::unordered_set<std::string> BLACKLISTED_HOSTS;
bool LOG_ON;
Logger LOGGER;