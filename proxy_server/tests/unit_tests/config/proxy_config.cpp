#include <gtest/gtest.h>
#include "config/proxy_config.hpp"
#include <fstream>
#include <filesystem>

class ProxyConfigTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // очищаем дефолтный конфиг файл если существует
        if(std::filesystem::exists("proxy_config.toml"))
        {
            std::filesystem::remove("proxy_config.toml");
        }
    }

    void TearDown() override
    {
        // очищаем дефолтный конфиг файл после теста
        if(std::filesystem::exists("proxy_config.toml"))
        {
            std::filesystem::remove("proxy_config.toml");
        }
    }
};

// тест загрузки дефолтных настроек
TEST_F(ProxyConfigTest, DefaultSettingsCreation)
{
    Proxy_Config config;
    const auto& settings = config.get_settings();
    
    EXPECT_EQ(settings.max_connections, 256);
    EXPECT_EQ(settings.timeout_milliseconds, 10000);
    EXPECT_EQ(settings.host, "0.0.0.0");
    EXPECT_EQ(settings.port, 12345);
    EXPECT_EQ(settings.log_on, false);
    EXPECT_EQ(settings.log_file_name, "proxy.log");
    EXPECT_EQ(settings.log_file_size_bytes, 1024 * 1024 * 16);
    EXPECT_EQ(settings.max_bandwidth_per_sec, 1024 * 1024 * 2);
}

// тест создания конфига с дефолтными значениями
TEST_F(ProxyConfigTest, CreateConfigWithDefaults)
{
    Proxy_Config config;
    const auto& settings = config.get_settings();
    
    EXPECT_GT(settings.max_connections, 0);
    EXPECT_GT(settings.timeout_milliseconds, 0);
    EXPECT_FALSE(settings.host.empty());
    EXPECT_GT(settings.port, 0);
    EXPECT_FALSE(settings.log_file_name.empty());
    EXPECT_GT(settings.log_file_size_bytes, 0);
}

// тест создания конфига по умолчанию при первом запуске
TEST_F(ProxyConfigTest, CreateDefaultConfigOnFirstRun)
{
    // конструктор вызывает load_or_create_cfg("proxy_config.toml")
    Proxy_Config config;
    
    // проверяем что файл создан
    EXPECT_TRUE(std::filesystem::exists("proxy_config.toml"));
    
    // проверяем дефолтные значения
    const auto& settings = config.get_settings();
    EXPECT_EQ(settings.max_connections, 256);
    EXPECT_EQ(settings.timeout_milliseconds, 10000);
    EXPECT_EQ(settings.host, "0.0.0.0");
    EXPECT_EQ(settings.port, 12345);
    EXPECT_EQ(settings.log_on, false);
    EXPECT_EQ(settings.log_file_name, "proxy.log");
    EXPECT_EQ(settings.log_file_size_bytes, 1024 * 1024 * 16);
    EXPECT_EQ(settings.max_bandwidth_per_sec, 1024 * 1024 * 2);
}

// тест загрузки существующего конфига
TEST_F(ProxyConfigTest, LoadExistingConfig)
{
    // создаём конфиг файл вручную
    std::ofstream file("proxy_config.toml");
    file << R"(
[proxy]
max_connections = 512
timeout_milliseconds = 20000
host = "192.168.1.100"
port = 8080
log_on = true
log_file_name = "test.log"
log_file_size_bytes = 8388608
max_bandwidth_per_sec = 5242880
)";
    file.close();
    
    // создаём объект который должен загрузить наш конфиг
    Proxy_Config config;
    const auto& settings = config.get_settings();
    
    EXPECT_EQ(settings.max_connections, 512);
    EXPECT_EQ(settings.timeout_milliseconds, 20000);
    EXPECT_EQ(settings.host, "192.168.1.100");
    EXPECT_EQ(settings.port, 8080);
    EXPECT_EQ(settings.log_on, true);
    EXPECT_EQ(settings.log_file_name, "test.log");
    EXPECT_EQ(settings.log_file_size_bytes, 8388608);
    EXPECT_EQ(settings.max_bandwidth_per_sec, 5242880);
}

// тест загрузки частичного конфига с дефолтными значениями
TEST_F(ProxyConfigTest, LoadPartialConfigWithDefaults)
{
    std::ofstream file("proxy_config.toml");
    file << R"(
[proxy]
max_connections = 128
port = 9999
)";
    file.close();
    
    Proxy_Config config;
    const auto& settings = config.get_settings();
    
    // проверяем загруженные значения
    EXPECT_EQ(settings.max_connections, 128);
    EXPECT_EQ(settings.port, 9999);
    
    // проверяем что остальные остались дефолтными
    EXPECT_EQ(settings.timeout_milliseconds, 10000);
    EXPECT_EQ(settings.host, "0.0.0.0");
    EXPECT_EQ(settings.log_on, false);
    EXPECT_EQ(settings.log_file_name, "proxy.log");
}

// тест обработки невалидного toml формата
TEST_F(ProxyConfigTest, HandleInvalidTOMLFormat)
{
    std::ofstream file("proxy_config.toml");
    file << "[proxy\nmax_connections = abc";
    file.close();
    
    testing::internal::CaptureStderr();
    Proxy_Config config;
    std::string output = testing::internal::GetCapturedStderr();
    
    // должна быть ошибка парсинга
    EXPECT_NE(output.find("TOML parsing error"), std::string::npos);
    EXPECT_NE(output.find("Using default settings"), std::string::npos);
    
    // проверяем что загрузились дефолтные значения
    const auto& settings = config.get_settings();
    EXPECT_EQ(settings.max_connections, 256);
    EXPECT_EQ(settings.port, 12345);
}

// тест обработки невалидных значений в конфиге
TEST_F(ProxyConfigTest, HandleInvalidConfigValues)
{
    std::ofstream file("proxy_config.toml");
    file << R"(
[proxy]
max_connections = 0
timeout_milliseconds = 0
host = ""
port = 0
log_file_name = ""
log_file_size_bytes = 0
)";
    file.close();
    
    testing::internal::CaptureStderr();
    Proxy_Config config;
    std::string output = testing::internal::GetCapturedStderr();
    
    // должны быть ошибки валидации
    EXPECT_NE(output.find("invalid"), std::string::npos);
    EXPECT_NE(output.find("using default"), std::string::npos);
    
    // проверяем что загрузились дефолтные значения
    const auto& settings = config.get_settings();
    EXPECT_EQ(settings.max_connections, 256);
    EXPECT_EQ(settings.timeout_milliseconds, 10000);
    EXPECT_FALSE(settings.host.empty());
    EXPECT_GT(settings.port, 0);
    EXPECT_FALSE(settings.log_file_name.empty());
    EXPECT_GT(settings.log_file_size_bytes, 0);
}

// тест геттера настроек
TEST_F(ProxyConfigTest, GetSettingsReturnsConstReference)
{
    Proxy_Config config;
    const auto& settings1 = config.get_settings();
    const auto& settings2 = config.get_settings();
    
    EXPECT_EQ(&settings1, &settings2);
}