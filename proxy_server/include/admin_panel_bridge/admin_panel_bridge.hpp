#include <boost/asio.hpp>
#include "api.hpp"

class AdminPanelBridge
{
    public:
        boost::asio::awaitable<void> run();
};