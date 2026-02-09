#include "proxy_facade/proxy_facade.hpp"
#include "globals/globals.hpp"

std::optional<std::vector<Session>> ProxyFacade::try_get_sessions() noexcept
{
    try
    {
        auto sessions = get_sessions();
        return sessions;
    }
    catch(...)
    {
        return std::nullopt;
    }
}

std::optional<Proxy_Config::Proxy_Settings> ProxyFacade::try_get_config() noexcept
{
    try
    {
        auto config = get_config();
        return config;
    }
    catch(...)
    {
        return std::nullopt;
    }
}

std::optional<std::string> ProxyFacade::try_get_log() noexcept
{
    try
    {
        auto log = get_log();
        return log;
    }
    catch(...)
    {
        return std::nullopt;
    }
}

std::vector<Session> ProxyFacade::get_sessions()
{
    std::vector<Session> sessions;
    sessions.reserve(__PROXY_GLOBALS__::SESSIONS->size());
    for(const auto& [ip, _] : *__PROXY_GLOBALS__::SESSIONS)
    {
        sessions.push_back(Session(ip));
    }
}

Proxy_Config::Proxy_Settings ProxyFacade::get_config()
{
    return __PROXY_GLOBALS__::PROXY_CONFIG;   
}

std::string ProxyFacade::get_log()
{

}