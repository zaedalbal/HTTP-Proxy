#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <map>
#include "user_traffic_manager.hpp"

class Server : public std::enable_shared_from_this<Server>
{
    public:
        Server(boost::asio::io_context& context, unsigned short port); // конструктор

        boost::asio::awaitable<void> run(); // запуск сервера

    private:
        boost::asio::awaitable<void> accept_connections(); // принимает соеденения, создает и запускает сессии

    private:
        unsigned short port_; // порт на котором работает сервер

        boost::asio::io_context& io_context_; // контекст boost asio

        boost::asio::ip::tcp::acceptor acceptor_; // acceptor для приема соеденений

        std::shared_ptr<User_traffic_manager> user_traffic_manager_;
};