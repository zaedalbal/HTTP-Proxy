#include "network/server.hpp"
#include "network/session.hpp"
#include "config/proxy_config.hpp"
#include "logger/logger.hpp"
#include <iostream>
#include <unordered_set>


Proxy_Config::Proxy_Settings PROXY_CONFIG;
std::unordered_set<std::string> BLACKLISTED_HOSTS; // глобальная хеш таблица, к которой идут обращения из других частей кода
bool LOG_ON;
Logger LOGGER; // объект класса Logger, через который происходит взаимодействие с логами из других частей кода
#ifdef DEBUG
Logger DEBUG_LOGGER;
#endif

int main(int argc, char** argv)
{
    try
    {
        // Загрузка конфигурации из proxy_config.toml
        Proxy_Config config;
        PROXY_CONFIG = config.get_settings();
        if(PROXY_CONFIG.blacklist_on)
            BLACKLISTED_HOSTS = config.get_blacklisted_hosts();
        LOG_ON = PROXY_CONFIG.log_on;
        LOGGER.init_logger(PROXY_CONFIG.log_file_name, PROXY_CONFIG.log_file_size_bytes);
#ifdef DEBUG
        // Если объявлен DEBUG, происходит объекта класса Logger через который происходит взаимодействие с дебаг логами
        DEBUG_LOGGER.init_logger(PROXY_CONFIG.log_file_name, PROXY_CONFIG.log_file_size_bytes);
        DEBUG_LOGGER.set_level(Logger::LOG_LEVEL::DEBUG);
#endif
        
        std::cout << "Starting proxy server on " << PROXY_CONFIG.host 
                  << ":" << PROXY_CONFIG.port << "...\n";
        std::cout << "Max connections: " << PROXY_CONFIG.max_connections << "\n";
        std::cout << "Timeout: " << PROXY_CONFIG.timeout_milliseconds << " milliseconds\n";
        std::cout << "Log on: " << PROXY_CONFIG.log_on << "\n";
        std::cout << "Log file name: " << PROXY_CONFIG.log_file_name << "\n";
        std::cout << "Log file size bytes: " << PROXY_CONFIG.log_file_size_bytes << "\n";
        std::cout << "Max_bandwidth_per_sec: " << PROXY_CONFIG.max_bandwidth_per_sec << " bytes\n";
        std::cout << "Blacklist_on: " << PROXY_CONFIG.blacklist_on << "\n";
        std::cout << "Blacklisted_hosts_file_name: " << PROXY_CONFIG.blacklisted_hosts_file_name << "\n";
        if(PROXY_CONFIG.blacklist_on)
        {
            if(!BLACKLISTED_HOSTS.empty())
            {
                std::cout << "Blacklisted hosts:\n";
                for(const auto& i : BLACKLISTED_HOSTS)
                {
                    std::cout << "\t\"" << i << "\"" << std::endl;
                }
            }
            else
                std::cout << "WARNING: Blacklist is enabled but no hosts were loaded!" << std::endl; 
        }
        boost::asio::io_context context;
        auto server = std::make_shared<Server>(context, PROXY_CONFIG.port);
        
        boost::asio::co_spawn(context, [server]() -> boost::asio::awaitable<void>
        {
            co_await server->run();
        }, boost::asio::detached);
        
        context.run();
    }
    catch(const std::exception& ex)
    {
        std::cerr << "\n!!!EXCEPTION IN MAIN FUNC: " << ex.what() << "!!!\n";
        return 1;
    }
    catch(...)
    {
        std::cerr << "\n!!!UNKNOWN EXCEPTION IN MAIN FUNC!!!\n";
        return 1;
    }

    return 0;
}