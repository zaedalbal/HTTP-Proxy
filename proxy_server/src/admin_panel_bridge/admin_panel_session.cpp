#include "admin_panel_bridge/admin_panel_session.hpp"
#include <memory>
#include <openssl/evp.h>
#include "globals/globals.hpp"

#define READ_BUFFER_SIZE 16384

AdminPanelSession::AdminPanelSession(boost::asio::ip::tcp::socket socket)
    : panel_socket_(std::move(socket))
{
}

boost::asio::awaitable<void> AdminPanelSession::start_session()
{
    co_await read_request();
}

boost::asio::awaitable<void> AdminPanelSession::read_request()
{
#ifdef DEBUG
    size_t i = 0;
#endif

    while(panel_socket_.is_open())
    {
#ifdef DEBUG
            __PROXY_GLOBALS__::DEBUG_LOGGER << "New iterations in AminPanelSession::read_request: i = " << i << std::endl;
            i++;
#endif
        api::RequestHeader header;
        boost::system::error_code ec;
        co_await boost::asio::async_read(panel_socket_, boost::asio::buffer(&header, sizeof(header)), 
        boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        if(ec)
        {
            panel_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            panel_socket_.close();
            co_return;
        }
        if(header.data_size > READ_BUFFER_SIZE*4)
        {
            panel_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            panel_socket_.close();
            co_return;
        }
        auto request = std::make_shared<api::Request>(header.data_size);
        request->Type = header.Type;
        request->id = header.id;
        request->Command = header.Command;
        request->isChunckedRequest = header.isChunckedRequest;
        if (header.data_size > 0)
        {
            co_await boost::asio::async_read(panel_socket_, boost::asio::buffer(request->data.get(), header.data_size),
            boost::asio::redirect_error(boost::asio::use_awaitable, ec));
            if(ec)
            {
                panel_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
                panel_socket_.close();
                co_return;
            }
        }
        if (!session_authenticated_)
        {
            if (request->Command != api::CommandName::AuthenticationRequest)
            {
                panel_socket_.close();
                co_return;
            }
            else
                co_await authorize(request);
        }
        else
            co_await hanlde_request(request);
    }
}

boost::asio::awaitable<std::optional<std::vector<uint8_t>>>
AdminPanelSession::get_hash_from_password(std::string_view password, std::span<const uint8_t> salt, int iterations, int hash_size)
{
    std::vector<uint8_t> hash(hash_size);
    if (PKCS5_PBKDF2_HMAC(password.data(),
                          static_cast<int>(password.size()),
                          salt.data(),
                          static_cast<int>(salt.size()),
                          iterations,
                          EVP_sha256(),
                          hash_size,
                          hash.data()) != 1)
        co_return std::nullopt;
    else
        co_return hash;
}

boost::asio::awaitable<void> AdminPanelSession::authorize(std::shared_ptr<api::Request> request)
{
    struct Authentication_header
    {
        std::uint32_t login_size;
        std::uint32_t password_size;
    };
    if(request->data_size < sizeof(Authentication_header))
        co_return;
    Authentication_header auth_head;
    std::memcpy(&auth_head, request->data.get(), sizeof(auth_head));
    if (auth_head.login_size > 1024 || auth_head.password_size > 1024)
        co_return;
    std::size_t need = sizeof(Authentication_header) + auth_head.login_size + auth_head.password_size;
    if(request->data_size < need)
        co_return;
    auto offset = sizeof(Authentication_header);
    std::string login(reinterpret_cast<char*>(request->data.get() + offset), auth_head.login_size);
    offset += auth_head.login_size;
    std::string password(reinterpret_cast<char*>(request->data.get() + offset), auth_head.password_size);
#ifdef DEBUG
    __PROXY_GLOBALS__::DEBUG_LOGGER << "password in AdminPanelSession::authorize : " << password << std::endl;
#endif
    auto account = Proxy_Config::find_admin_panel_account_by_login(login, __PROXY_GLOBALS__::PROXY_CONFIG.admin_panel_auth_data_file_name);
    if(account.has_value())
    {
        auto acc = account.value();
        auto hash_check = co_await get_hash_from_password(password, acc.salt, acc.iterations, acc.hash.size());
        if(hash_check.has_value())
        {
            if(acc.hash == hash_check.value())
            {
                auto response = std::make_shared<api::Response>();
                response->RequestCommand = api::CommandName::AuthenticationRequest;
                response->ResponseCommand = api::CommandName::AuthenticationResponseSuccess;
                response->id = request->id;
                response->ProxyStatus = true;
                response->isChunckedResponse = false;
                response->data_size = 0;
                session_authenticated_ = true;
                co_await send_response(response);
                if(__PROXY_GLOBALS__::LOG_ON)
                    __PROXY_GLOBALS__::LOGGER << "Successful login to the panel: username: " << login << std::endl;
                co_return;
            }
        }
    }
    auto response = std::make_shared<api::Response>();
    response->RequestCommand = api::CommandName::AuthenticationRequest;
    response->ResponseCommand = api::CommandName::AuthenticationResponseError;
    response->ProxyStatus = true;
    response->isChunckedResponse = false;
    response->data_size = 0;
    if(__PROXY_GLOBALS__::LOG_ON)
        __PROXY_GLOBALS__::LOGGER << "Unsuccessful login to the panel from:" << panel_socket_.remote_endpoint().address().to_string() << std::endl;
    co_await send_response(response);
    boost::system::error_code ec;
    panel_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    panel_socket_.close();
}

boost::asio::awaitable<void> AdminPanelSession::hanlde_request(std::shared_ptr<api::Request> request)
{
    auto response = std::make_shared<api::Response>();
    switch(request->Command)
    {
// фигурные скобки нужны внутри для использования auto (НЕ УБИРАТЬ ИХ В БУДУЩЕМ!!!)
    case api::CommandName::Get_proxy_config:
    {
        auto try_config = Facade_.try_get_config();
        if(try_config.has_value())
        {
            auto config = try_config.value();
            response->data_size = sizeof(api::Proxy_Settings);
            response->data = std::make_unique<char[]>(response->data_size);
            api::Proxy_Settings proxy_settings_to_send;
            proxy_settings_to_send.blacklist_on = config.blacklist_on;
            proxy_settings_to_send.log_on = config.log_on;
            proxy_settings_to_send.admin_panel_on = config.admin_panel_on;
            proxy_settings_to_send.port = config.port;
            proxy_settings_to_send.admin_panel_port = config.admin_panel_port;
            proxy_settings_to_send.max_connections = config.max_connections;
            proxy_settings_to_send.timeout_milliseconds = config.timeout_milliseconds;
            proxy_settings_to_send.log_file_size_bytes = config.log_file_size_bytes;
            proxy_settings_to_send.max_bandwidth_per_sec = config.max_bandwidth_per_sec;
            std::memcpy(response->data.get(), &proxy_settings_to_send, sizeof(api::Proxy_Settings));
        }
        else
            response->RequestFailed = true;
    break;
    }
    
    case api::CommandName::Get_proxy_sessions:
    {
        auto try_sessions = Facade_.try_get_sessions();
        if(try_sessions.has_value())
        {
            auto sessions = try_sessions.value();
            response->data_size = sessions.size() * sizeof(Session);
            response->data = std::make_unique<char[]>(response->data_size);
            std::memcpy(response->data.get(), sessions.data(), response->data_size);
        }
        else
            response->RequestFailed = true;
    break;
    }

    default:
        response->RequestFailed = true;
        break;
    }
    response->id = request->id;
    response->ResponseCommand = api::CommandName::NONE_COMMAND;
    response->RequestCommand = request->Command;
    co_await send_response(response);
    co_return;
}

boost::asio::awaitable<void> AdminPanelSession::send_response(std::shared_ptr<api::Response> response)
{
    boost::system::error_code ec;

    api::ResponseHeader header;
    header.id = response->id;
    header.data_size = response->data_size;
    header.ResponseCommand = response->ResponseCommand;
    header.RequestCommand = response->RequestCommand;
    header.ProxyStatus = response->ProxyStatus;
    header.isChunckedResponse = response->isChunckedResponse;
    header.RequestFailed = response->RequestFailed;

    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(&header, sizeof(header)));
    if(response->data_size > 0)
        buffers.push_back(boost::asio::buffer(response->data.get(), response->data_size));

    co_await boost::asio::async_write
    (panel_socket_, buffers, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    if(ec)
    {
        panel_socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        panel_socket_.close();
        co_return;
    }
    co_return;
}