#include "../include/session.hpp"
#include "../include/analyze_request.hpp"
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <iostream>
#include <atomic>
#include <array>

Session::Session(boost::asio::ip::tcp::socket socket) : client_socket_(std::move(socket)),
upstream_(client_socket_.get_executor())
{}

boost::asio::awaitable<void> Session::start_session()
{
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
            {
                co_await https_handler(result.host, result.port);
                co_return;
            }
            else
                co_await http_handler(result.host, result.port, req);
        }
        else
            co_await send_bad_request("BAD REQUEST");
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Exception in handle request: " << ex.what();
    }
}

boost::asio::awaitable<void> Session::send_bad_request(const std::string str)
{
    boost::beast::http::response<boost::beast::http::string_body> res(boost::beast::http::status::bad_request, 11);
    res.set(boost::beast::http::field::server, "Proxy");
    res.set(boost::beast::http::field::content_type, "text/plain");
    res.body() = str;
    res.prepare_payload();
    co_await boost::beast::http::async_write(client_socket_, res, boost::asio::use_awaitable);
    co_return;
}

boost::asio::awaitable<void> Session::http_handler
(const std::string& host, const std::string& port, const boost::beast::http::request<boost::beast::http::string_body> request)
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

boost::asio::awaitable<void> Session::https_handler (const std::string& host, const std::string& port)
{
    boost::asio::ip::tcp::resolver resolver(client_socket_.get_executor());
    boost::system::error_code ec;
    auto upstream_ptr = std::make_shared<boost::asio::ip::tcp::socket>(client_socket_.get_executor());
    auto finished = std::make_shared<std::atomic_bool>(false);
    auto self = shared_from_this();

    auto close_both = [self, upstream_ptr]()
    {
        boost::system::error_code ec;
        self->client_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        upstream_ptr->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        self->client_socket_.close(ec);
        upstream_ptr->close(ec);
    };

    auto client_to_server = [self, upstream_ptr, finished, close_both] -> boost::asio::awaitable<void>
    {
        std::array<char, TUNNEL_BUFFER_SIZE> read_buffer_from_client;
        boost::system::error_code ec;
        for(;;)
        {
            if(finished->load())
                break;
            auto bytes_transferred = co_await self->client_socket_.async_read_some
            (boost::asio::buffer(read_buffer_from_client), boost::asio::redirect_error(boost::asio::use_awaitable, ec));
            if(bytes_transferred == 0 || ec)
                break;
            co_await boost::asio::async_write
            (*upstream_ptr, boost::asio::buffer(read_buffer_from_client, bytes_transferred), boost::asio::redirect_error(boost::asio::use_awaitable, ec));
            if(ec)
                break;
        }
        if(ec)
            std::cerr << "Error in client_to_server: " << ec.what() << std::endl;
        if(!finished->exchange(true))
            close_both();
    };

    auto server_to_client = [self, upstream_ptr, finished, close_both]() -> boost::asio::awaitable<void>
    {
        std::array<char, TUNNEL_BUFFER_SIZE> read_buffer_from_server;
        boost::system::error_code ec;
        for(;;)
        {
            if(finished->load())
                break;
            auto bytes_transferred = co_await upstream_ptr->async_read_some
            (boost::asio::buffer(read_buffer_from_server), boost::asio::redirect_error(boost::asio::use_awaitable, ec));
            if(bytes_transferred == 0 || ec)
                break;
            co_await boost::asio::async_write
            (self->client_socket_, boost::asio::buffer(read_buffer_from_server, bytes_transferred), boost::asio::redirect_error(boost::asio::use_awaitable, ec));
            if(ec)
                break;
        }
        if(ec)
            std::cerr << "Error in server_to_client: " << ec.what() << std::endl;
        if(!finished->exchange(true))
            close_both();
    };
    auto results = co_await resolver.async_resolve(host, port, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    co_await boost::asio::async_connect(*upstream_ptr, results, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    if(ec)
    {
        std::cerr << "Error in connect to upstream: " << ec.what() << std::endl;
        co_await send_bad_request(ec.what());
        co_return;
    }
    boost::beast::http::response<boost::beast::http::empty_body> res(boost::beast::http::status::ok, 11);
    res.reason("Connection Established");
    res.prepare_payload();
    co_await boost::beast::http::async_write(client_socket_, res, boost::asio::use_awaitable);
    boost::asio::co_spawn(client_socket_.get_executor(), client_to_server, boost::asio::detached);
    boost::asio::co_spawn(client_socket_.get_executor(), server_to_client, boost::asio::detached);
    co_return;
}