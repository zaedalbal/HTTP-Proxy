#pragma once
#include "traffic_limiter.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <thread>

class User_traffic_manager
{
    public:
        std::shared_ptr<Traffic_limiter> get_or_create_user(const std::string& ip);

    private:
        std::unordered_map<std::string, std::weak_ptr<Traffic_limiter>> users_;

        std::mutex mutex_;
};