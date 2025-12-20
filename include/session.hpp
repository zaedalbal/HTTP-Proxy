#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>

#define TUNNEL_BUFFER_SIZE 16184

class Session : public std::enable_shared_from_this<Session>
{
    public:
        Session(boost::asio::ip::tcp::socket socket);
        
        boost::asio::awaitable<void> start_session();
    private:
        boost::asio::awaitable<void> handle_request();

        boost::asio::awaitable<void> send_bad_request(const std::string str);

        boost::asio::awaitable<void> http_handler
        (const std::string& host, const std::string& port,
        const boost::beast::http::request<boost::beast::http::string_body> request);

        boost::asio::awaitable<void> https_handler
        (const std::string& host, const std::string& port);


    private:
        boost::asio::ip::tcp::socket client_socket_;
};