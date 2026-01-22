// в этом файле объявляются все глобальные данные

#include "config/proxy_config.hpp"
#include "logger/logger.hpp"
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace __PROXY_GLOBALS__
{
    Proxy_Config::Proxy_Settings PROXY_CONFIG;
    std::unordered_set<std::string> BLACKLISTED_HOSTS; // глобальная хеш таблица, к которой идут обращения из других частей кода

    bool LOG_ON;

    Logger LOGGER; // объект класса Logger, через который происходит взаимодействие с логами из других частей кода
    Logger DEBUG_LOGGER;

    std::atomic<size_t> ACTIVE_CONNECTIONS; // счетчик активных соеденений
    std::mutex ACTIVE_CONNECTIONS_MUTEX; // мьютекс для проверки активных соеднений
    std::condition_variable ACTIVE_CONNECTIONS_COND_VAR; // сcond variable для работы с кол-ом активных соеденений
}