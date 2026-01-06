#include "network/session.hpp"
#include "network/analyze_request.hpp"
#include <boost/asio/experimental/awaitable_operators.hpp>
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
        std::cerr << "Exception in handle request: " << ex.what();
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
    co_return;
}

boost::asio::awaitable<void> Session::http_handler
(const std::string& host, const std::string& port, const boost::beast::http::request<boost::beast::http::string_body> request)
{
    boost::asio::ip::tcp::resolver resolver(client_socket_.get_executor());
    boost::system::error_code ec;
    auto upstream_ptr = std::make_shared<boost::asio::ip::tcp::socket>(client_socket_.get_executor()); // сокет для соеденения с сервером
    auto finished = std::make_shared<std::atomic_bool>(false); // флаг завершения
    auto self = shared_from_this(); // shared_ptr для того чтобы объект не уничтожился
    
    auto results = co_await resolver.async_resolve(
        host, port, 
        boost::asio::redirect_error(boost::asio::use_awaitable, ec)); 
    co_await boost::asio::async_connect( // подключение к серверу
        *upstream_ptr, results, 
        boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    if(ec)
    {
#ifdef DEBUG
        std::cerr << "Error in connect to upstream: " << ec.what() << std::endl;
#endif
        co_await send_bad_request(ec.what());
        co_return;
    }
    auto close_both = [self, upstream_ptr]() // закрытие обоих сокетов
    {
        boost::system::error_code ec;
        self->client_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        upstream_ptr->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        self->client_socket_.close(ec);
        upstream_ptr->close(ec);
    };
    const size_t MAX_BODY_SIZE = 1024 * 1024 * 64; // лимит телв 64мб
    if(request.body().size() > MAX_BODY_SIZE) // если тело больше чем 64мб, послать BAD REQUEST и закрыть сокеты
    {
#ifdef DEBUG
        std::cerr << "Error: HTTP request body too large!" << std::endl;
#endif
        co_await send_bad_request("Error: HTTP request too large!");
        close_both();
        co_return;
    }
    
    auto request_str = std::make_shared<std::string>();
    std::ostringstream oss;
    oss << request;             // преобразование запроса в строку,
    *request_str = oss.str();   // чтобы можно было контроллировать скольк байт передается
    
    // корутина для отправки запроса от клиента к серверу
    auto client_to_server = [self, upstream_ptr, finished, close_both, request_str]() -> boost::asio::awaitable<void>
    {
        boost::system::error_code ec;
        auto bytes_transferred = request_str->size(); // размер в байтах всего request'а
        std::size_t offset = 0; // смещение в буфере
        while(offset < bytes_transferred)
        {
            auto allowed = self->traffic_limiter_->acquire(bytes_transferred - offset);
            if(allowed == 0) // ждать 10мс пока токены не обновятся
            {
                boost::asio::steady_timer timer(self->client_socket_.get_executor());
                timer.expires_after(std::chrono::milliseconds(10));
                co_await timer.async_wait(boost::asio::use_awaitable);
                continue;
            }
            auto sent = co_await boost::asio::async_write(
                *upstream_ptr,
                boost::asio::buffer(request_str->data() + offset, allowed),
                boost::asio::redirect_error(boost::asio::use_awaitable, ec));
            if(ec)
                break;
            offset += sent;
        }
        
#ifdef DEBUG
        if(ec)
            std::cerr << "Error in client_to_server: " << ec.what() << std::endl;
#endif
        
        if(!finished->exchange(true));
    };

    // корутина для отправки ответа клиенту от сервера
    auto server_to_client = [self, upstream_ptr, finished, close_both]() -> boost::asio::awaitable<void>
    {
        boost::system::error_code ec;
        boost::beast::flat_buffer read_buffer;
        boost::beast::http::response<boost::beast::http::string_body> res;

        co_await boost::beast::http::async_read(
            *upstream_ptr, 
            read_buffer, 
            res,
            boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        
        if(ec)  
        {
#ifdef DEBUG
        std::cerr << "Error reading response from upstream: " << ec.what() << std::endl;
#endif
            if(!finished->exchange(true))
            co_return;
        }
        const size_t MAX_BODY_SIZE = 1024 * 1024 * 64; // лимит тела 64мб
        if(res.body().size() > MAX_BODY_SIZE) // если тело больше чем 64мб, отправить BAD REQUEST
    {
#ifdef DEBUG
        std::cerr << "Error: HTTP response body too large!" << std::endl;
#endif
        if(!finished->exchange(true))
        co_return;
    }
        auto response_str = std::make_shared<std::string>();
        std::ostringstream oss;
        oss << res;                 // преобразование в ответа в строку,
        *response_str = oss.str();  // чтобы можно было контроллировать сколько байт передается
        auto bytes_transferred = response_str->size(); // размер в байтах всего ответа
        std::size_t offset = 0; // смещение в буфере
        while(offset < bytes_transferred)
        {
            auto allowed = self->traffic_limiter_->acquire(bytes_transferred - offset);
            if(allowed == 0) // ждать 10мс пока токены не обновсятся
            {
                boost::asio::steady_timer timer(self->client_socket_.get_executor());
                timer.expires_after(std::chrono::milliseconds(10));
                co_await timer.async_wait(boost::asio::use_awaitable);
                continue;
            }
            auto sent = co_await boost::asio::async_write(
                self->client_socket_,
                boost::asio::buffer(response_str->data() + offset, allowed),
                boost::asio::redirect_error(boost::asio::use_awaitable, ec));
            if(ec)
                break;
            offset += sent;
        }
#ifdef DEBUG
    if(ec)
        std::cerr << "Error in server_to_client: " << ec.what() << std::endl;
#endif
        if(!finished->exchange(true)) // если данная корутина завершилась первой, то закрыть сокеты
            close_both();
    };
    co_await (boost::asio::experimental::awaitable_operators::operator&&(client_to_server(), server_to_client())); // запуск корутин
    co_return;
}

boost::asio::awaitable<void> Session::https_handler (const std::string& host, const std::string& port)
{
    boost::asio::ip::tcp::resolver resolver(client_socket_.get_executor());
    boost::system::error_code ec;
    auto upstream_ptr = std::make_shared<boost::asio::ip::tcp::socket>(client_socket_.get_executor()); // сокет для соеденения с сервером
    auto finished = std::make_shared<std::atomic_bool>(false); // флаг завершения
    auto self = shared_from_this(); // shared_ptr, чтобы объект не уничтожился раньше чем надо

    auto close_both = [self, upstream_ptr]() // закрытие обоих сокетов
    {
        boost::system::error_code ec;
        self->client_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        upstream_ptr->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        self->client_socket_.close(ec);
        upstream_ptr->close(ec);
    };

    // корутина для отправки данных от клиента к серверу
    auto client_to_server = [self, upstream_ptr, finished, close_both] -> boost::asio::awaitable<void>
    {
        std::array<char, TUNNEL_BUFFER_SIZE> read_buffer_from_client; // буфер для чтения
        boost::system::error_code ec;
        for(;;)
        {
            if(finished->load())
                break;
            auto bytes_transferred = co_await self->client_socket_.async_read_some
            (boost::asio::buffer(read_buffer_from_client), boost::asio::redirect_error(boost::asio::use_awaitable, ec));
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
                if(ec)
                    break;
                offset += sent;
            }
        }
#ifdef DEBUG
        if(ec)
            std::cerr << "Error in client_to_server: " << ec.what() << std::endl;
#endif
        if(!finished->exchange(true)) // если данная корутина завершилась первой, то закрыть сокеты
            close_both();
    };

    // корутина для отправки данных от сервера к клиенту
    auto server_to_client = [self, upstream_ptr, finished, close_both]() -> boost::asio::awaitable<void>
    {
        std::array<char, TUNNEL_BUFFER_SIZE> read_buffer_from_server; // буфер для чтения
        boost::system::error_code ec;
        for(;;)
        {
            if(finished->load())
                break;
            auto bytes_transferred = co_await upstream_ptr->async_read_some
            (boost::asio::buffer(read_buffer_from_server), boost::asio::redirect_error(boost::asio::use_awaitable, ec));
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
                if(ec)
                    break;
                offset += sent;
            }
        }
#ifdef DEBUG
        if(ec)
            std::cerr << "Error in server_to_client: " << ec.what() << std::endl;
#endif
        if(!finished->exchange(true)) // если данная корутина завершилась первой, то закрыть сокеты
            close_both();
    };

    auto results = co_await resolver.async_resolve(host, port, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    // подключение к серверу
    co_await boost::asio::async_connect(*upstream_ptr, results, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    if(ec)
    {
#ifdef DEBUG
        std::cerr << "Error in connect to upstream: " << ec.what() << std::endl;
#endif
        co_await send_bad_request(ec.what());
        co_return;
    }
    boost::beast::http::response<boost::beast::http::empty_body> res(boost::beast::http::status::ok, 11);
    res.reason("Connection Established");
    res.prepare_payload();
    // отправка подтеврждения, что тунель установлен
    co_await boost::beast::http::async_write(client_socket_, res, boost::asio::use_awaitable);
    // запуск корутин
    co_await (boost::asio::experimental::awaitable_operators::operator&&(client_to_server(), server_to_client()));
    co_return;
}