#include <string>

class Proxy_Config
{
    public:
        Proxy_Config(); // конструктор

        struct Proxy_Settings // настройки конфига
        {
            int max_connections = 256;
            int timeout_seconds = 60; // пока что не используется

            std::string host = "0.0.0.0";
            unsigned short port = 12345;

            bool log_on = false; // пока что не используется
            std::string log_file_name = "proxy.log"; // пока что не используется
        };
        
        const Proxy_Settings& get_settings() const; // геттер для получение конфига

    private:
        Proxy_Settings settings; // текущий конфиг

    private:
        bool validate() const; // проверка корректности конфига

        void load_or_create_cfg(const std::string& filename); // загрузка или создание конфига
};