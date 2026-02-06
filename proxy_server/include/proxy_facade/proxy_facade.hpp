#include <vector>
#include "proxy_facade/session_struct.hpp"
#include "config/proxy_config.hpp"

class ProxyFacade // через данный класс происходит запрос/изменение состояния сервера
{
    public:
        ProxyFacade();
        
        std::vector<Session> get_sessions();

        Proxy_Config::Proxy_Settings get_config();

        std::string get_log();

    private:

};