#include "admin_panel_bridge/admin_panel_session.hpp"

AdminPanelSession::AdminPanelSession(boost::asio::ip::tcp::socket socket)
: panel_socket_(std::move(socket))
{}

boost::asio::awaitable<void> AdminPanelSession::start_session()
{
    co_await authorize();
}