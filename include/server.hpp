#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>

class Server : public std::enable_shared_from_this<Server>
{
    public:
        Server(boost::asio::io_context& context, unsigned short port);

        boost::asio::awaitable<void> run(); // запуск сервера

    private:
        boost::asio::awaitable<void> accept_connections(); // принимает соеденения, создает и запускает сессии

    private:
        unsigned short port_;

        boost::asio::io_context& io_context_;

        boost::asio::ip::tcp::acceptor acceptor_;
};