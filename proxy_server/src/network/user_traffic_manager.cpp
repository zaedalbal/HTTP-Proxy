#include "network/user_traffic_manager.hpp"
#include "globals/globals.hpp"

std::shared_ptr<Traffic_limiter> User_traffic_manager::get_or_create_user(const std::string& ip)
{
    std::lock_guard lock(mutex_);
    auto it = users_.find(ip);
    if(it != users_.end())
    {
        if(auto limiter = it->second.lock())
            return limiter;
        else
            users_.erase(it);
    }
    auto limiter = std::make_shared<Traffic_limiter>(__PROXY_GLOBALS__::PROXY_CONFIG.max_bandwidth_per_sec);
    users_.emplace(ip, limiter);
    return limiter;
}

User_traffic_manager::User_traffic_manager()
{}

User_traffic_manager::~User_traffic_manager()
{}