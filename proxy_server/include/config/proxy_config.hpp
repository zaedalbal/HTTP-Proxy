#pragma once
#include <string>
#include <unordered_set>

class Proxy_Config
{
    public:
        Proxy_Config(); // конструктор

        struct Proxy_Settings // настройки конфига
        {
            int64_t max_connections = 256; // пока что не используется
            int64_t timeout_milliseconds = 10000;
            // int64_t из за того что toml не хочет принимать std::size_t

            std::string host = "0.0.0.0"; // пока что не используется
            unsigned short port = 12345;

            bool log_on = false;
            std::string log_file_name = "proxy.log";
            int64_t log_file_size_bytes = 1024 * 1024 * 16; // 16 мб по дефолту

            int64_t max_bandwidth_per_sec = 1024 * 1024 * 2; // 2 мб/сек по дефолту

            bool blacklist_on = false;
            std::string blacklisted_hosts_file_name = "blacklisted_hosts.toml";
        };
        
        const Proxy_Settings& get_settings() const {return settings;}; // геттер для получение конфига

        std::unordered_set<std::string> get_blacklisted_hosts() const;

    private:
        Proxy_Settings settings; // текущий конфиг

    private:
        bool validate() const; // проверка корректности конфига

        void load_or_create_cfg(const std::string& filename); // загрузка или создание конфига
};