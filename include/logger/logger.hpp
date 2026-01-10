#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/core.hpp>

class Logger
{
    public:
        enum LOG_LEVEL{INFO, DEBUG};

        Logger(const std::string& log_file_name, std::size_t rotation_size_bytes);

        template<typename T>
        Logger& operator<<(const T& data)
        {
            stream_ << data;
            return *this;
        }
        Logger& operator<<(std::ostream& (*func)(std::ostream&))
        {
            if(func == static_cast<std::ostream&(*)(std::ostream&)>(std::endl))
                flush();
            return *this;
        }


        void set_level(LOG_LEVEL log_level);
    
    private:
        void init_logger(const std::string& log_file, std::size_t rotation_size);

        void flush();

    private:
        std::ostringstream stream_;
        LOG_LEVEL log_level_;
};