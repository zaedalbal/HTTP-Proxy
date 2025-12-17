#include "../include/server.hpp"
#include "../include/session.hpp"
#include <iostream>

Server::Server(boost::asio::io_context& context, unsigned short port)
: io_context_(context), port_(port), acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_))
{}

boost::asio::awaitable<void> Server::run()
{
    co_await accept_connections();
}

boost::asio::awaitable<void> Server::accept_connections()
{
    for(;;)
    {
        try
        {
            auto socket = co_await acceptor_.async_accept(boost::asio::use_awaitable);
            auto session = std::make_shared<Session>(std::move(socket));
            boost::asio::co_spawn(io_context_, [session]()->boost::asio::awaitable<void>
            {
                co_await session->start_session();
            }, boost::asio::detached);
        }
        catch(const std::exception& ex)
        {
            std::cout << "Exception in acception: " << ex.what() << std::endl;
        }
    }
}