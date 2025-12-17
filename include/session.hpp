#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>

class Session : public std::enable_shared_from_this<Session>
{
    public:
        Session(boost::asio::ip::tcp::socket socket);
        
        boost::asio::awaitable<void> start_session();
    private:
        boost::asio::awaitable<void> handle_request();

        boost::asio::awaitable<void> send_bad_request();

        boost::asio::awaitable<void> http_handler
        (std::string host, std::string port,
        boost::beast::http::request<boost::beast::http::string_body> request);

        boost::asio::awaitable<void> https_handler
        (std::string host, std::string port,
        boost::beast::http::request<boost::beast::http::string_body> request);


    private:
        boost::asio::ip::tcp::socket client_socket_;

        boost::beast::tcp_stream upstream_;
};