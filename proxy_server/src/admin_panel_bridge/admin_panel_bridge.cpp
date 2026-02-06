#include "globals/globals.hpp"
#include "admin_panel_bridge/admin_panel_bridge.hpp"
#include "admin_panel_bridge/admin_panel_session.hpp"

AdminPanelBridge::AdminPanelBridge(boost::asio::io_context& context, unsigned short port)
: io_context_(context), port_(port),
acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_)){}

boost::asio::awaitable<void> AdminPanelBridge::run()
{
    if(__PROXY_GLOBALS__::LOG_ON)
        __PROXY_GLOBALS__::LOGGER << "Starting admin panel bridge" << std::endl;
    co_await accept_connections();
}

boost::asio::awaitable<void> AdminPanelBridge::accept_connections()
{
    boost::system::error_code ec;
    for(;;)
    {
        try
        {
            auto socket = co_await acceptor_.async_accept(boost::asio::use_awaitable);
            if(__PROXY_GLOBALS__::LOG_ON)
                __PROXY_GLOBALS__::LOGGER << "New connection in admin panel bridge: " << socket.remote_endpoint().address() << std::endl;
            auto session = std::make_shared<AdminPanelSession>(std::move(socket));
            boost::asio::co_spawn(io_context_, [session]()->boost::asio::awaitable<void>
            {
                co_await session->start_session();
            }, boost::asio::detached);
        }
        catch(const std::exception& ex)
        {
#ifdef DEBUG
            DEBUG_LOGGER << "Exception in acception: " << ex.what() << std::endl;
#endif
        }
    }
}