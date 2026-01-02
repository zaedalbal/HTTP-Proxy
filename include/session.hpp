#pragma once
#include "traffic_limiter.hpp"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <chrono>

#define TUNNEL_BUFFER_SIZE 16184

class Session : public std::enable_shared_from_this<Session>
{
    public:
        Session(boost::asio::ip::tcp::socket socket); // конструктор
        
        boost::asio::awaitable<void> start_session(); // начало сессии
    private:
        boost::asio::awaitable<void> handle_request(); // обработка запроса

        boost::asio::awaitable<void> send_bad_request(const std::string str); // отправка страницы при некорректном запросе

        boost::asio::awaitable<void> http_handler // обработка http соеденения
        (const std::string& host, const std::string& port,
        const boost::beast::http::request<boost::beast::http::string_body> request);

        boost::asio::awaitable<void> https_handler // обработа https соеденения
        (const std::string& host, const std::string& port);


    private:
        boost::asio::ip::tcp::socket client_socket_; // сокет клиента

        traffic_limiter traffic_limiter_; // лимитер трафика
};