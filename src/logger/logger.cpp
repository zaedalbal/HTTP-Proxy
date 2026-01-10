#include "logger/logger.hpp"

Logger::Logger(const std::string& log_file_name, std::size_t rotation_size_bytes) : log_level_(LOG_LEVEL::INFO)
{
    init_logger(log_file_name, rotation_size_bytes);
}

void Logger::set_level(LOG_LEVEL level)
{
    log_level_ = level;
}

void Logger::init_logger(const std::string& log_file, std::size_t rotation_size)
{
    boost::log::add_file_log
    (
        boost::log::keywords::file_name = log_file,
        boost::log::keywords::rotation_size = rotation_size,
        boost::log::keywords::format = "[%TimeStamp%] [%Severity%] %Message%"
    );
    boost::log::add_common_attributes();
}

void Logger::flush()
{
    switch(log_level_)
    {
        case LOG_LEVEL::INFO:
            BOOST_LOG_TRIVIAL(info) << stream_.str();
            break;
        case LOG_LEVEL::DEBUG:
            BOOST_LOG_TRIVIAL(debug) << stream_.str();
            break;
    }
    stream_.str(""); // очистка
    stream_.clear();
}