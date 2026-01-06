#include "network/server.hpp"
#include "network/session.hpp"
#include <iostream>

Server::Server(boost::asio::io_context& context, unsigned short port)
: io_context_(context), port_(port),
acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_)),
user_traffic_manager_(std::make_shared<User_traffic_manager>())
{}

boost::asio::awaitable<void> Server::run()
{
    co_await accept_connections();
}

boost::asio::awaitable<void> Server::accept_connections()
{
    boost::system::error_code ec;
    for(;;)
    {
        try
        {
            auto socket = co_await acceptor_.async_accept(boost::asio::use_awaitable);
            auto session = std::make_shared<Session>(std::move(socket), user_traffic_manager_);
            boost::asio::co_spawn(io_context_, [session]()->boost::asio::awaitable<void>
            {
                co_await session->start_session();
            }, boost::asio::detached);
        }
        catch(const std::exception& ex)
        {
#ifdef DEBUG
            std::cout << "Exception in acception: " << ex.what() << std::endl;
#endif
        }
    }
}