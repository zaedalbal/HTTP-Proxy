#include "../include/server.hpp"
#include "../include/session.hpp"
#include <iostream>

#define PORT 12345

int main(int argc, char** argv)
{
    try
    {
        boost::asio::io_context context;
        auto server = std::make_shared<Server>(context, PORT);
        boost::asio::co_spawn(context, [server]()->boost::asio::awaitable<void>
    {
        co_await server->run();
    }, boost::asio::detached);
    context.run();
    }
    catch(...)
    {
        std::cerr << "\n!!!EXCEPTION IN MAIN FUNC!!!\n";
        return 1;
    }
    return 0;
}