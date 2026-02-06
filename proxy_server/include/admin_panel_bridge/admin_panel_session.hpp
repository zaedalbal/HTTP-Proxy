#pragma once
#include <boost/asio.hpp>
#include "api.hpp"

class AdminPanelSession
{
    public:
        AdminPanelSession(boost::asio::ip::tcp::socket socket); // конструктор

        boost::asio::awaitable<void> start_session();

    private:
        boost::asio::awaitable<void> authorize();
        
        boost::asio::awaitable<void> read_request();

        boost::asio::awaitable<void> hanlde_request(std::shared_ptr<api::Request> request);

        boost::asio::awaitable<void> send_response(api::Response response);

    private:
        boost::asio::ip::tcp::socket panel_socket_;

        bool session_authenticated_ = false;
};