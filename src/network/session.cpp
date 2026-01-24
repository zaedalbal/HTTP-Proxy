#include "network/session.hpp"
#include "network/analyze_request.hpp"
#include "globals/globals.hpp"
#include <boost/asio/experimental/awaitable_operators.hpp>
#include "utils/timer.hpp"
#include <iostream>
#include <atomic>
#include <array>
#include <sstream>


Session::Session(boost::asio::ip::tcp::socket socket, std::shared_ptr<User_traffic_manager> manager)
: client_socket_(std::move(socket))
{
    auto ep = client_socket_.remote_endpoint(); // получение endpoint'а
    auto client_ip = ep.address().to_string(); // строка с ip адресом
    traffic_limiter_ = manager->get_or_create_user(client_ip);  // получение или создание пользователя с помощью Traffic Manager'а
}

boost::asio::awaitable<void> Session::start_session() // старт сессии
{
    co_await handle_request(); // запуск обработчика request'ов
}

Session::~Session()
{}

boost::asio::awaitable<void> Session::handle_request()
{
    try
    {
        boost::beast::flat_buffer read_buffer;
        boost::beast::http::request<boost::beast::http::string_body> req;
        boost::system::error_code ec;
        co_await boost::beast::http::async_read(client_socket_, read_buffer, req, 
        boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        if(!ec) // если нет ошибки
        {
            auto result = HttpHandler::analyze_request(req); // анализ запроса
            if(__PROXY_GLOBALS__::LOG_ON)
                __PROXY_GLOBALS__::LOGGER << "Request from " << client_socket_.remote_endpoint().address() << ":\n" 
                << req << std::endl;
            if(result.is_blacklisted)
            {
                co_await send_bad_request("BLACKLISTED HOST");
                co_return;
            }
            if(result.is_connect) // если CONNECT, то вызвать https_handler
            {
                co_await https_handler(result.host, result.port);
                co_return;
            }
            else // иначе вызвать http_hanlder
                co_await http_handler(result.host, result.port, req);
        }
        else // если ошибка, то послать BAD REQUEST
            co_await send_bad_request("BAD REQUEST");
    }
    catch(const std::exception& ex)
    {
#ifdef DEBUG
        __PROXY_GLOBALS__::DEBUG_LOGGER << "Exception in handle request: " << ex.what();
#endif
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
    if(__PROXY_GLOBALS__::LOG_ON)
        __PROXY_GLOBALS__::LOGGER << "Send bad request to " << client_socket_.remote_endpoint().address() << ":\n" 
        << res << std::endl;
    co_return;
}

boost::asio::awaitable<void> Session::http_handler
(const std::string& host, const std::string& port, const boost::beast::http::request<boost::beast::http::string_body> request)
{
    auto executor = client_socket_.get_executor();
    boost::asio::ip::tcp::resolver resolver(executor);
    boost::system::error_code ec;
    auto upstream_ptr = std::make_shared<boost::asio::ip::tcp::socket>(executor);
    auto finished = std::make_shared<std::atomic_bool>(false);
    auto self_weak = weak_from_this();
    auto timer = std::make_shared<Timer>(executor, __PROXY_GLOBALS__::PROXY_CONFIG.timeout_milliseconds);
    
    auto close_both = [self_weak, upstream_ptr, timer]()
    {
        if(auto self = self_weak.lock())
        {
            timer->stop();
            boost::system::error_code ec;
            self->client_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            upstream_ptr->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            self->client_socket_.close(ec);
            upstream_ptr->close(ec);
        }
        else
            return;
    };

    timer->set_callback_func([upstream_ptr](){upstream_ptr->close();});
    timer->start();
    auto results = co_await resolver.async_resolve(host, port, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    timer->refresh();
    co_await boost::asio::async_connect(*upstream_ptr, results, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    timer->refresh();
    if(ec)
    {
        timer->stop();
#ifdef DEBUG
        __PROXY_GLOBALS__::DEBUG_LOGGER << "Error in connect to upstream: " << ec.what() << std::endl;
#endif
        co_await send_bad_request(ec.what());
        co_return;
    }

    auto modified_req = request; // модификация запроса для upstream сервера
    std::string target = std::string(request.target()); // конвертация url
    auto scheme_pos = target.find("://");
    if(scheme_pos != std::string::npos)
    {
        auto path_pos = target.find('/', scheme_pos + 3);
        if(path_pos != std::string::npos)
            target = target.substr(path_pos);
        else
            target = "/";
    }
    modified_req.target(target);
    modified_req.erase(boost::beast::http::field::proxy_connection); // удаление proxy-connection заголовка

    // отправка модифицированного запроса на upstream сервер
    co_await boost::beast::http::async_write(*upstream_ptr, modified_req, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    timer->refresh();
    if(ec)
    {
        timer->stop();
#ifdef DEBUG
        __PROXY_GLOBALS__::DEBUG_LOGGER << "Error writing to upstream: " << ec.what() << std::endl;
#endif
        co_await send_bad_request(ec.what());
        co_return;
    }
    timer->set_callback_func([finished](){finished->store(true);});

    // корутина для чтения от сервера и отправки клиенту
    auto server_to_client = [self_weak, upstream_ptr, finished, close_both, timer]() -> boost::asio::awaitable<void>
    {
        if(auto self = self_weak.lock())
        {
            std::array<char, TUNNEL_BUFFER_SIZE> read_buffer_from_server;
            boost::system::error_code ec;
            for(;;)
            {
                if(finished->load())
                    break;
                auto bytes_transferred = co_await upstream_ptr->async_read_some
                (boost::asio::buffer(read_buffer_from_server), boost::asio::redirect_error(boost::asio::use_awaitable, ec));
                timer->refresh();
                if(bytes_transferred == 0 || ec)
                    break;
                std::size_t offset = 0;
                while(offset < bytes_transferred)
                {
                    auto allowed = self->traffic_limiter_->acquire(bytes_transferred - offset);
                    if(allowed == 0)
                    {
                        boost::asio::steady_timer wait_timer(self->client_socket_.get_executor());
                        wait_timer.expires_after(std::chrono::milliseconds(10));
                        co_await wait_timer.async_wait(boost::asio::use_awaitable);
                        continue;
                    }
                    
                    auto sent = co_await boost::asio::async_write
                    (self->client_socket_,
                    boost::asio::buffer(read_buffer_from_server.data() + offset, allowed),
                    boost::asio::redirect_error(boost::asio::use_awaitable, ec));
                    timer->refresh();
                    
                    if(ec)
                        break;
                    offset += sent;
                }
            }
            
#ifdef DEBUG
            if(ec)
                __PROXY_GLOBALS__::DEBUG_LOGGER << "Error in server_to_client (HTTP): " << ec.what() << std::endl;
#endif
            if(!finished->exchange(true))
                close_both();
        }
        else
            co_return;
    };

    // корутина для чтения от клиента и отправки серверу
    auto client_to_server = [self_weak, upstream_ptr, finished, close_both, timer]() -> boost::asio::awaitable<void>
    {
        if(auto self = self_weak.lock())
        {
            std::array<char, TUNNEL_BUFFER_SIZE> read_buffer_from_client;
            boost::system::error_code ec;
            for(;;)
            {
                if(finished->load())
                    break;
                auto bytes_transferred = co_await self->client_socket_.async_read_some
                (boost::asio::buffer(read_buffer_from_client), boost::asio::redirect_error(boost::asio::use_awaitable, ec));
                timer->refresh();
                if(bytes_transferred == 0 || ec)
                    break;
                std::size_t offset = 0;
                while(offset < bytes_transferred)
                {
                    auto allowed = self->traffic_limiter_->acquire(bytes_transferred - offset);
                    if(allowed == 0)
                    {
                        boost::asio::steady_timer wait_timer(self->client_socket_.get_executor());
                        wait_timer.expires_after(std::chrono::milliseconds(10));
                        co_await wait_timer.async_wait(boost::asio::use_awaitable);
                        continue;
                    }
                    auto sent = co_await boost::asio::async_write
                    (*upstream_ptr,
                    boost::asio::buffer(read_buffer_from_client.data() + offset, allowed),
                    boost::asio::redirect_error(boost::asio::use_awaitable, ec));
                    timer->refresh();
                    if(ec)
                        break;
                    offset += sent;
                }
            }
#ifdef DEBUG
            if(ec)
                __PROXY_GLOBALS__::DEBUG_LOGGER << "Error in client_to_server (HTTP): " << ec.what() << std::endl;
#endif
            if(!finished->exchange(true))
                close_both();
        }
        else
            co_return;
    };
    // запуск обеих корутин для двунаправленной передачи
    co_await (boost::asio::experimental::awaitable_operators::operator&&(client_to_server(), server_to_client()));
    co_return;
}

boost::asio::awaitable<void> Session::https_handler (const std::string& host, const std::string& port)
{
    auto executor = client_socket_.get_executor();
    boost::asio::ip::tcp::resolver resolver(executor);
    boost::system::error_code ec;
    auto upstream_ptr = std::make_shared<boost::asio::ip::tcp::socket>(executor); // сокет для соеденения с сервером
    auto finished = std::make_shared<std::atomic_bool>(false); // флаг завершения
    auto self_weak = weak_from_this(); // shared_ptr, чтобы объект не уничтожился раньше чем надо
    auto timer = std::make_shared<Timer>(executor, __PROXY_GLOBALS__::PROXY_CONFIG.timeout_milliseconds);

    auto close_both = [self_weak, upstream_ptr, timer]() // закрытие обоих сокетов
    {
        if(auto self = self_weak.lock())
        {
            timer->stop();
            boost::system::error_code ec;
            self->client_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            upstream_ptr->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            self->client_socket_.close(ec);
            upstream_ptr->close(ec);
        }
        else
            return;
    };

    // корутина для отправки данных от клиента к серверу
    auto client_to_server = [self_weak, upstream_ptr, finished, close_both, timer] -> boost::asio::awaitable<void>
    {
        if(auto self = self_weak.lock())
        {
            std::array<char, TUNNEL_BUFFER_SIZE> read_buffer_from_client; // буфер для чтения
            boost::system::error_code ec;
            for(;;)
            {
                if(finished->load())
                    break;
                auto bytes_transferred = co_await self->client_socket_.async_read_some
                (boost::asio::buffer(read_buffer_from_client), boost::asio::redirect_error(boost::asio::use_awaitable, ec));
                timer->refresh(); // обновление таймера
                if(bytes_transferred == 0 || ec)
                    break;
                std::size_t offset = 0; // смещение в буфере
                while(offset < bytes_transferred)
                {
                    auto allowed = self->traffic_limiter_->acquire(bytes_transferred - offset);
                    if(allowed == 0) // ждать 10 мс пока токены не обновятся
                    {
                        boost::asio::steady_timer timer(self->client_socket_.get_executor());
                        timer.expires_after(std::chrono::milliseconds(10));
                        co_await timer.async_wait(boost::asio::use_awaitable);
                        continue;
                    }
                    auto sent = co_await boost::asio::async_write
                    (*upstream_ptr,
                    boost::asio::buffer(read_buffer_from_client.data() + offset, allowed),
                    boost::asio::redirect_error(boost::asio::use_awaitable, ec));
                    timer->refresh();
                    if(ec)
                        break;
                    offset += sent;
                }
            }
#ifdef DEBUG
        if(ec)
            DEBUG_LOGGER << "Error in client_to_server: " << ec.what() << std::endl;
#endif
            if(!finished->exchange(true)) // если данная корутина завершилась первой, то закрыть сокеты
                close_both();
        }
        else
            co_return;
    };

    // корутина для отправки данных от сервера к клиенту
    auto server_to_client = [self_weak, upstream_ptr, finished, close_both, timer]() -> boost::asio::awaitable<void>
    {
        if(auto self = self_weak.lock())
        {
            std::array<char, TUNNEL_BUFFER_SIZE> read_buffer_from_server; // буфер для чтения
            boost::system::error_code ec;
            for(;;)
            {
                if(finished->load())
                    break;
                auto bytes_transferred = co_await upstream_ptr->async_read_some
                (boost::asio::buffer(read_buffer_from_server), boost::asio::redirect_error(boost::asio::use_awaitable, ec));
                timer->refresh();
                if(bytes_transferred == 0 || ec)
                    break;
                std::size_t offset = 0; // смещение в буфере
                while(offset < bytes_transferred)
                {
                    auto allowed = self->traffic_limiter_->acquire(bytes_transferred - offset);
                    if(allowed == 0) // ждать 10 мс пока токены не обновятся
                    {
                        boost::asio::steady_timer timer(self->client_socket_.get_executor());
                        timer.expires_after(std::chrono::milliseconds(10));
                        co_await timer.async_wait(boost::asio::use_awaitable);
                        continue;
                    }
                    auto sent = co_await boost::asio::async_write
                    (self->client_socket_,
                    boost::asio::buffer(read_buffer_from_server.data() + offset, allowed),
                    boost::asio::redirect_error(boost::asio::use_awaitable, ec));
                    timer->refresh();
                    if(ec)
                        break;
                    offset += sent;
                }
            }
#ifdef DEBUG
        if(ec)
            DEBUG_LOGGER << "Error in server_to_client: " << ec.what() << std::endl;
#endif
            if(!finished->exchange(true)) // если данная корутина завершилась первой, то закрыть сокеты
                close_both();
        }
        else
            co_return;
    };
    timer->set_callback_func([upstream_ptr](){upstream_ptr->close();}); // колбэк для подключения и резолвинга
    timer->start(); // запуск таймера
    auto results = co_await resolver.async_resolve(host, port, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    timer->refresh(); // обновление таймера
    // подключение к серверу
    co_await boost::asio::async_connect(*upstream_ptr, results, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    timer->refresh();
    if(ec)
    {
        timer->stop();
#ifdef DEBUG
        DEBUG_LOGGER << "Error in connect to upstream: " << ec.what() << std::endl;
#endif
        co_await send_bad_request(ec.what());
        co_return;
    }
    boost::beast::http::response<boost::beast::http::empty_body> res(boost::beast::http::status::ok, 11);
    res.reason("Connection Established");
    res.prepare_payload();
    // отправка подтеврждения, что тунель установлен
    co_await boost::beast::http::async_write(client_socket_, res, boost::asio::use_awaitable);
    timer->refresh();
    timer->set_callback_func([finished](){finished->store(true);}); // колбэк для корутин
    // запуск корутин
    co_await (boost::asio::experimental::awaitable_operators::operator&&(client_to_server(), server_to_client()));
    co_return;
}