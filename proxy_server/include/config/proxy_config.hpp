#pragma once
#include <string>
#include <unordered_set>
#include <cstdint>
#include <optional>
#include <vector>

class Proxy_Config
{
    public:
        Proxy_Config(); // конструктор

        struct Proxy_Settings // настройки конфига
        {
            int64_t max_connections = 256;
            int64_t timeout_milliseconds = 10000;
            // int64_t из за того что toml не хочет принимать std::size_t

            std::string host = "0.0.0.0";
            unsigned short port = 12345;

            bool log_on = false;
            std::string log_file_name = "proxy.log";
            int64_t log_file_size_bytes = 1024 * 1024 * 16; // 16 мб по дефолту

            int64_t max_bandwidth_per_sec = 1024 * 1024 * 2; // 2 мб/сек по дефолту

            bool blacklist_on = false;
            std::string blacklisted_hosts_file_name = "blacklisted_hosts.toml";

            bool admin_panel_on = false;
            unsigned short admin_panel_port = 54321;
            std::string admin_panel_auth_data_file_name = "admin_panel_auth_data_file_name.toml";
        };

        struct Admin_panel_account
        {
            std::string login;
            std::string algorithm;
            uint32_t iterations;
            std::vector<uint8_t> salt;
            std::vector<uint8_t> hash;
        };

        const Proxy_Settings& get_settings() const {return settings;}; // геттер для получение конфига

        std::unordered_set<std::string> get_blacklisted_hosts() const;

        static std::optional<Proxy_Config::Admin_panel_account> find_admin_panel_account_by_login(const std::string& login, const std::string& filename);

    private:
        Proxy_Settings settings; // текущий конфиг

    private:
        bool validate() const; // проверка корректности конфига

        void load_or_create_cfg(const std::string& filename); // загрузка или создание конфига
};