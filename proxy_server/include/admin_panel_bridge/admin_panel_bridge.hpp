#include <boost/asio.hpp>
#include "api.hpp"

class AdminPanelBridge
{
    public:
        boost::asio::awaitable<void> run();
    
    private:
        boost::asio::awaitable<void> authorize();

        boost::asio::awaitable<void> read_request();
        
        boost::asio::awaitable<void> send_response();
};