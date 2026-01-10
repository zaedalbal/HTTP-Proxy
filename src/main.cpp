#include "network/server.hpp"
#include "network/session.hpp"
#include "config/proxy_config.hpp"
#include "logger/logger.hpp"
#include <iostream>


Proxy_Config::Proxy_Settings PROXY_CONFIG;
Logger LOGGER;
Logger DEBUG_LOGGER;

int main(int argc, char** argv)
{
    try
    {
        // Загрузка конфигурации из proxy_config.toml
        Proxy_Config config;
        PROXY_CONFIG = config.get_settings();
        
        std::cout << "Starting proxy server on " << PROXY_CONFIG.host 
                  << ":" << PROXY_CONFIG.port << "...\n";
        std::cout << "Max connections: " << PROXY_CONFIG.max_connections << "\n";
        std::cout << "Timeout: " << PROXY_CONFIG.timeout_milliseconds << " milliseconds\n";
        std::cout << "Max_bandwidth_per_sec: " << PROXY_CONFIG.max_bandwidth_per_sec << " bytes\n";
        
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