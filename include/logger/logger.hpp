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

        Logger() = default;

        template<typename T>
        Logger& operator<<(const T& data)
        {
            stream_ << data;
            return *this;
        }
        Logger& operator<<(std::ostream& (*func)(std::ostream&))
        { // на будущее: на компиляторах отличных от gcc все может работать не так как надо (из за сравнения указателей)
            if(func == static_cast<std::ostream&(*)(std::ostream&)>(std::endl))
                flush();
            else
                stream_ << func;
            return *this;
        }

        void set_level(LOG_LEVEL log_level);

        void init_logger(const std::string& log_file, std::size_t rotation_size);
    
    private:

        void flush();

    private:
        std::ostringstream stream_;
        LOG_LEVEL log_level_;
};