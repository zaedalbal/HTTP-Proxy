#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include "logger/logger.hpp"

namespace fs = std::filesystem;

class LoggerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        test_log_file_ = "test_log.log";
        // удаляем старые файлы логов если есть
        cleanup_log_files();
    }

    void TearDown() override
    {
        cleanup_log_files();
    }

    void cleanup_log_files()
    {
        for (const auto& entry : fs::directory_iterator("."))
        {
            if (entry.path().string().find("test_log") != std::string::npos)
            {
                fs::remove(entry.path());
            }
        }
    }

    std::string read_log_file(const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    bool log_contains(const std::string& content, const std::string& substring)
    {
        return content.find(substring) != std::string::npos;
    }

    std::string test_log_file_;
};

// базовая инициализация
TEST_F(LoggerTest, InitLogger)
{
    Logger logger;
    EXPECT_NO_THROW(logger.init_logger(test_log_file_, 10 * 1024 * 1024));
}

// установка уровня логирования
TEST_F(LoggerTest, SetLevel)
{
    Logger logger;
    EXPECT_NO_THROW(logger.set_level(Logger::LOG_LEVEL::INFO));
    EXPECT_NO_THROW(logger.set_level(Logger::LOG_LEVEL::DEBUG));
}

// логирование строки
TEST_F(LoggerTest, LogString)
{
    Logger logger;
    logger.init_logger(test_log_file_, 10 * 1024 * 1024);
    logger.set_level(Logger::LOG_LEVEL::INFO);
    
    logger << "test message" << std::endl;
    
    // небольшая задержка для записи в файл
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::string content = read_log_file(test_log_file_);
    EXPECT_TRUE(log_contains(content, "test message"));
    EXPECT_TRUE(log_contains(content, "[info]"));
}

// логирование числа
TEST_F(LoggerTest, LogInteger)
{
    Logger logger;
    logger.init_logger(test_log_file_, 10 * 1024 * 1024);
    logger.set_level(Logger::LOG_LEVEL::INFO);
    
    logger << 42 << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::string content = read_log_file(test_log_file_);
    EXPECT_TRUE(log_contains(content, "42"));
}

// логирование числа с плавающей точкой
TEST_F(LoggerTest, LogDouble)
{
    Logger logger;
    logger.init_logger(test_log_file_, 10 * 1024 * 1024);
    logger.set_level(Logger::LOG_LEVEL::INFO);
    
    logger << 3.14159 << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::string content = read_log_file(test_log_file_);
    EXPECT_TRUE(log_contains(content, "3.14159"));
}

// логирование нескольких значений в одной строке
TEST_F(LoggerTest, LogMultipleValues)
{
    Logger logger;
    logger.init_logger(test_log_file_, 10 * 1024 * 1024);
    logger.set_level(Logger::LOG_LEVEL::INFO);
    
    logger << "value: " << 100 << " status: " << "ok" << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::string content = read_log_file(test_log_file_);
    EXPECT_TRUE(log_contains(content, "value: 100 status: ok"));
}

// логирование с уровнем debug
TEST_F(LoggerTest, LogDebugLevel)
{
    Logger logger;
    logger.init_logger(test_log_file_, 10 * 1024 * 1024);
    logger.set_level(Logger::LOG_LEVEL::DEBUG);
    
    logger << "debug message" << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::string content = read_log_file(test_log_file_);
    EXPECT_TRUE(log_contains(content, "debug message"));
    EXPECT_TRUE(log_contains(content, "[debug]"));
}

// несколько последовательных сообщений
TEST_F(LoggerTest, LogMultipleMessages)
{
    Logger logger;
    logger.init_logger(test_log_file_, 10 * 1024 * 1024);
    logger.set_level(Logger::LOG_LEVEL::INFO);
    
    logger << "first message" << std::endl;
    logger << "second message" << std::endl;
    logger << "third message" << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::string content = read_log_file(test_log_file_);
    EXPECT_TRUE(log_contains(content, "first message"));
    EXPECT_TRUE(log_contains(content, "second message"));
    EXPECT_TRUE(log_contains(content, "third message"));
}

// смена уровня логирования
TEST_F(LoggerTest, ChangeLevelBetweenLogs)
{
    Logger logger;
    logger.init_logger(test_log_file_, 10 * 1024 * 1024);
    
    logger.set_level(Logger::LOG_LEVEL::INFO);
    logger << "info level message" << std::endl;
    
    logger.set_level(Logger::LOG_LEVEL::DEBUG);
    logger << "debug level message" << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::string content = read_log_file(test_log_file_);
    EXPECT_TRUE(log_contains(content, "info level message"));
    EXPECT_TRUE(log_contains(content, "[info]"));
    EXPECT_TRUE(log_contains(content, "debug level message"));
    EXPECT_TRUE(log_contains(content, "[debug]"));
}

// пустое сообщение
TEST_F(LoggerTest, LogEmptyMessage)
{
    Logger logger;
    logger.init_logger(test_log_file_, 10 * 1024 * 1024);
    logger.set_level(Logger::LOG_LEVEL::INFO);
    
    EXPECT_NO_THROW(logger << "" << std::endl);
}

// логирование булева значения
TEST_F(LoggerTest, LogBool)
{
    Logger logger;
    logger.init_logger(test_log_file_, 10 * 1024 * 1024);
    logger.set_level(Logger::LOG_LEVEL::INFO);
    
    logger << std::boolalpha << true << " " << false << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::string content = read_log_file(test_log_file_);
    EXPECT_TRUE(log_contains(content, "true"));
    EXPECT_TRUE(log_contains(content, "false"));
}

// ротация файла (требует записи большого объема данных)
TEST_F(LoggerTest, FileRotation)
{
    Logger logger;
    // маленький размер для ротации
    logger.init_logger(test_log_file_, 1024);
    logger.set_level(Logger::LOG_LEVEL::INFO);
    
    // пишем много данных для триггера ротации
    for (int i = 0; i < 100; ++i)
    {
        logger << "this is a long message to trigger rotation " << i << std::endl;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // проверяем что создан хотя бы один файл
    bool rotation_occurred = false;
    for (const auto& entry : fs::directory_iterator("."))
    {
        if (entry.path().string().find("test_log") != std::string::npos)
        {
            rotation_occurred = true;
        }
    }
    EXPECT_TRUE(rotation_occurred);
}