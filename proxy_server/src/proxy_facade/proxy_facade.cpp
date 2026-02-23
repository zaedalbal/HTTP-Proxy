#include "proxy_facade/proxy_facade.hpp"
#include "globals/globals.hpp"
#include <fstream>

ProxyFacade::ProxyFacade()
{}

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
    if(!__PROXY_GLOBALS__::SESSIONS)
        throw std::runtime_error("__PROXY_GLOBALS__::SESSION = nullptr");
    std::vector<Session> sessions;
    sessions.reserve(__PROXY_GLOBALS__::SESSIONS->size());
    for(const auto& [ip, _] : *__PROXY_GLOBALS__::SESSIONS)
    {
        sessions.push_back(Session(ip));
    }
    return sessions;
}

Proxy_Config::Proxy_Settings ProxyFacade::get_config()
{
    return __PROXY_GLOBALS__::PROXY_CONFIG;   
}

std::string ProxyFacade::get_log()
{
    const std::size_t bytes_to_read = 16384;
    std::string log;
    log.resize(bytes_to_read);
    std::ifstream file(__PROXY_GLOBALS__::PROXY_CONFIG.log_file_name);
    if(!file)    
        throw std::runtime_error("Cant't open file");
    file.read(log.data(), bytes_to_read);
    file.close();
    return log;
}