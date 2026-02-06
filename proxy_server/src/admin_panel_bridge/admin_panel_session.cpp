#include "admin_panel_bridge/admin_panel_session.hpp"
#include <memory>

#define READ_BUFFER_SIZE 16384

AdminPanelSession::AdminPanelSession(boost::asio::ip::tcp::socket socket)
: panel_socket_(std::move(socket))
{}

boost::asio::awaitable<void> AdminPanelSession::start_session()
{
    co_await read_request();
}

boost::asio::awaitable<void> AdminPanelSession::read_request()
{
    api::RequestHeader header;
    boost::system::error_code ec;
    co_await boost::asio::async_read
    (panel_socket_, boost::asio::buffer(&header, sizeof(header)), boost::asio::use_awaitable);
    auto request = std::make_shared<api::Request>(header.data_size);
    request->Type = header.Type;
    request->id = header.id;
    request->Command = header.Command;
    request->isChunckedRequest = header.isChunckedRequest;
    if(header.data_size > 0)
    {
        co_await boost::asio::async_read
        (panel_socket_, boost::asio::buffer(request->data.get(), header.data_size), boost::asio::use_awaitable);
    }
    if(!session_authenticated_)
    {
        if(request->Command != api::CommandName::AuthenticationRequest)
        {
            panel_socket_.close();
            co_return;
        }
        else
            co_await authorize();
    }
    else
        hanlde_request(request);
}