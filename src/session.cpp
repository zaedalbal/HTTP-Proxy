#include "../include/session.hpp"
#include "../include/analyze_request.hpp"
#include <iostream>

Session::Session(boost::asio::ip::tcp::socket socket) : client_socket_(std::move(socket)),
upstream_(client_socket_.get_executor())
{}

boost::asio::awaitable<void> Session::start_session()
{
    for(;client_socket_.is_open();)
        co_await handle_request();
}

boost::asio::awaitable<void> Session::handle_request()
{
    try
    {
        boost::beast::flat_buffer read_buffer;
        boost::beast::http::request<boost::beast::http::string_body> req;
        boost::system::error_code ec;
        co_await boost::beast::http::async_read(client_socket_, read_buffer, req, 
        boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        if(!ec)
        {
            auto result = HttpHandler::analyze_request(req);
            if(result.is_connect)
                co_await send_bad_request(); // пока что метода для https нет
            else
                co_await http_handler(std::move(result.host), std::move(result.port), std::move(req));
        }
        else
            co_await send_bad_request();
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Exception in handle request: " << ex.what();
    }
}

boost::asio::awaitable<void> Session::send_bad_request()
{
    boost::beast::http::response<boost::beast::http::string_body> res(boost::beast::http::status::bad_request, 11);
    res.set(boost::beast::http::field::server, "Proxy");
    res.set(boost::beast::http::field::content_type, "text/plain");
    res.body() = "BAD REQUEST";
    res.prepare_payload();
    co_await boost::beast::http::async_write(client_socket_, res, boost::asio::use_awaitable);
    co_return;
}

boost::asio::awaitable<void> Session::http_handler
(std::string host, std::string port, boost::beast::http::request<boost::beast::http::string_body> request)
{
    try
    {
        boost::beast::flat_buffer read_buffer_;
        boost::beast::http::response<boost::beast::http::string_body> response;
        boost::asio::ip::tcp::resolver resolver(client_socket_.get_executor());
        auto results = co_await resolver.async_resolve(host, port, boost::asio::use_awaitable);
        co_await upstream_.async_connect(results, boost::asio::use_awaitable);
        co_await boost::beast::http::async_write(upstream_, request, boost::asio::use_awaitable);
        co_await boost::beast::http::async_read(upstream_, read_buffer_, response, boost::asio::use_awaitable);
        co_await boost::beast::http::async_write(client_socket_, response, boost::asio::use_awaitable);
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }
}