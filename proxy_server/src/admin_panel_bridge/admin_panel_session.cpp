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
    while(true)
    {
        api::RequestHeader header;
        boost::system::error_code ec;
        co_await boost::asio::async_read(panel_socket_, boost::asio::buffer(&header, sizeof(header)), 
        boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        if(ec)
        {
            panel_socket_.close();
            co_return;
        }
        if(header.data_size > READ_BUFFER_SIZE*4)
        {
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

boost::asio::awaitable<std::optional<std::string>> AdminPanelSession::get_hash_from_password(std::string_view password, std::string_view salt, int iterations, int hash_size)
{
    std::string hash;
    hash.resize(hash_size);
    auto *salt_bytes = reinterpret_cast<const unsigned char *>(salt.data());
    if (PKCS5_PBKDF2_HMAC(password.data(),
                          static_cast<int>(password.size()),
                          salt_bytes,
                          static_cast<int>(salt.size()),
                          iterations,
                          EVP_sha256(),
                          hash_size,
                          reinterpret_cast<unsigned char *>(hash.data())) != 1)
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
                response->Command = api::CommandName::AuthenticationResponseSuccess;
                response->ProxyStatus = true;
                response->isChunckedResponse = false;
                response->data_size = 0;
                session_authenticated_ = true;
                co_await send_response(response);
                co_return;
            }
        }
    }
    auto response = std::make_shared<api::Response>();
    response->Command = api::CommandName::AuthenticationResponseError;
    response->ProxyStatus = true;
    response->isChunckedResponse = false;
    response->data_size = 0;
    co_await send_response(response); 
}

boost::asio::awaitable<void> AdminPanelSession::hanlde_request(std::shared_ptr<api::Request> request)
{
    co_return;
}

boost::asio::awaitable<void> AdminPanelSession::send_response(std::shared_ptr<api::Response> response)
{
    boost::system::error_code ec;
    std::size_t total_size = sizeof(api::ResponseHeader) + response->data_size;
    co_await boost::asio::async_write
    (panel_socket_, boost::asio::buffer(response.get(), total_size), boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    if(ec)
    {
        panel_socket_.close();
        co_return;
    }
    co_return;
}