#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "api.hpp"

class AdminPanelBridge
{
    public:
        AdminPanelBridge(boost::asio::io_context& context, unsigned short port); // конструктор
        
        boost::asio::awaitable<void> run();
    
    private:
        boost::asio::awaitable<void> accept_connections();

    private:
        unsigned short port_;

        boost::asio::io_context& io_context_;

        boost::asio::ip::tcp::acceptor acceptor_;

        boost::asio::ssl::context ssl_context_;
};