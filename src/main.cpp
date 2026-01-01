#include "../include/server.hpp"
#include "../include/session.hpp"
#include <iostream>
#include <cstdlib>

#define DEFAULT_PORT 12345

int main(int argc, char** argv)
{
    try
    {
        unsigned short port = DEFAULT_PORT; // изначально дефолтный порт
        if(argc > 1) // если указан аргумент, то использовать его как порт
        {
            char* end;
            long parsed_port = std::strtol(argv[1], &end, 10);
            if(*end != '\0' || parsed_port < 1 || parsed_port > 65535)
            {
                std::cerr << "Error: Invalid port number. Must be between 1 and 65535.\n";
                std::cerr << "Usage: " << argv[0] << " [port]\n";
                return 1;
            }
            port = static_cast<unsigned short>(parsed_port);
        }
        std::cout << "Starting proxy server on port " << port << "...\n";
        boost::asio::io_context context;
        auto server = std::make_shared<Server>(context, port); // создание сервера
        boost::asio::co_spawn(context, [server]() -> boost::asio::awaitable<void>
        {
            co_await server->run();
        }, boost::asio::detached);
        context.run(); // запуск сервера
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