#include <vector>
#include "proxy_facade/session_struct.hpp"
#include "config/proxy_config.hpp"

class ProxyFacade // через данный класс происходит запрос/изменение состояния сервера
{
    public:
        ProxyFacade();

        // Разделение методов на публичные и привытные - задел на будущее, если добавлять права доступа
        // делать проверку в публичных, если все ок, то вызывать приватные
        
        std::optional<std::vector<Session>> try_get_sessions() noexcept;

        std::optional<Proxy_Config::Proxy_Settings> try_get_config() noexcept;

        std::optional<std::string> try_get_log() noexcept;

    private:
        std::vector<Session> get_sessions();

        Proxy_Config::Proxy_Settings get_config();

        std::string get_log();
};