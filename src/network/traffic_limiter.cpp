#include "network/traffic_limiter.hpp"
#include "globals/globals.hpp"

Traffic_limiter::Traffic_limiter(uint64_t bytes_per_sec)
{
    __PROXY_GLOBALS__::ACTIVE_CONNECTIONS++;
    max_tokens_ = bytes_per_sec * 1.5; // 2 мб по дефолту
    tokens_ = max_tokens_;
    rate_bytes_per_sec_ = bytes_per_sec; // 1.5 мб/сек по дефолту
    last_update_ = std::chrono::steady_clock::now();
}

void Traffic_limiter::refill()
{
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration<double>(now - last_update_).count(); // сколько секунд прошло с последнего обновления
    tokens_ = std::min(max_tokens_, tokens_ + static_cast<std::size_t>(elapsed * rate_bytes_per_sec_)); // сколько байт надо добавить
    last_update_ = now;
}

std::size_t Traffic_limiter::acquire(std::size_t want)
{
    std::lock_guard lock(mutex_);
    refill();
    std::size_t allowed = std::min(tokens_, want);
    tokens_ -= allowed;
    return allowed;
}

Traffic_limiter::~Traffic_limiter()
{
    __PROXY_GLOBALS__::ACTIVE_CONNECTIONS--;
}