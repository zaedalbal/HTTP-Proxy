#pragma once
#include <string>

class Proxy_Config
{
    public:
        Proxy_Config(); // конструктор

        struct Proxy_Settings // настройки конфига
        {
            int64_t max_connections = 256; // пока что не используется
            int64_t timeout_milliseconds = 10000;

            std::string host = "0.0.0.0"; // пока что не используется
            unsigned short port = 12345;

            bool log_on = false; // пока что не используется
            std::string log_file_name = "proxy.log"; // пока что не используется
        };
        
        const Proxy_Settings& get_settings() const {return settings;}; // геттер для получение конфига

    private:
        Proxy_Settings settings; // текущий конфиг

    private:
        bool validate() const; // проверка корректности конфига

        void load_or_create_cfg(const std::string& filename); // загрузка или создание конфига
};