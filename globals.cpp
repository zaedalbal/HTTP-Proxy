// в этом файле объявляются все глобальные данные

#include "config/proxy_config.hpp"
#include "logger/logger.hpp"

Proxy_Config::Proxy_Settings PROXY_CONFIG;
std::unordered_set<std::string> BLACKLISTED_HOSTS; // глобальная хеш таблица, к которой идут обращения из других частей кода
bool LOG_ON;
Logger LOGGER; // объект класса Logger, через который происходит взаимодействие с логами из других частей кода
#ifdef DEBUG
Logger DEBUG_LOGGER;
#endif