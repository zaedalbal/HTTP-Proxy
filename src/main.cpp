#include "network/server.hpp"
#include "network/session.hpp"
#include "config/proxy_config.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    try
    {
        // Загрузка конфигурации из proxy_config.toml
        Proxy_Config config;
        const auto& settings = config.get_settings();
        
        std::cout << "Starting proxy server on " << settings.host 
                  << ":" << settings.port << "...\n";
        std::cout << "Max connections: " << settings.max_connections << "\n";
        std::cout << "Timeout: " << settings.timeout_seconds << " seconds\n";
        
        boost::asio::io_context context;
        auto server = std::make_shared<Server>(context, settings.port);
        
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