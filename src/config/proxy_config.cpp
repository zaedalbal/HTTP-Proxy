#include "config/proxy_config.hpp"
#include <toml++/toml.hpp>
#include <fstream>
#include <iostream>

Proxy_Config::Proxy_Config()
{
    load_or_create_cfg("proxy_config.toml");
}

bool Proxy_Config::validate() const
{
    bool error_flag = false;
    if(settings.max_connections < 1)
    {
        std::cerr << "Error in config: max_connection must be at lest 1" << std::endl;
        error_flag = true;
    }
    if(settings.timeout_milliseconds <= 0 || settings.timeout_milliseconds > 600000)
    {
        std::cerr << "Error in config: timeout_milliseconds must be in range 1-600000" << std::endl;
        error_flag = true;
    }
    if(settings.host.empty())
    {
        std::cerr << "Error in config: host cannot be empty" << std::endl;
        error_flag = true;
    }
    if(settings.port < 1)
    {
        std::cerr << "Error in config: port must be greater than 0" << std::endl;
        error_flag = true;
    }
    if(settings.log_file_name.empty())
    {
        std::cerr << "Error in config: log_file_name cannot be empty" << std::endl;
        error_flag = true;
    }
    if(settings.log_file_size_bytes < 1)
    {
        std::cerr << "Error in config: log_file_size_bytes must be greater than 0" << std::endl;
        error_flag = true;
    }
    if(error_flag)
        return false;
    else
        return true;
}

void Proxy_Config::load_or_create_cfg(const std::string& filename)
{
    try
    {
        std::ifstream file(filename);
        if(file.good())
        {
            file.close();
            auto config = toml::parse_file(filename);
            if(config["proxy"])
            {
                auto proxy = config["proxy"];
                settings.max_connections = proxy["max_connections"].value_or(settings.max_connections);
                settings.timeout_milliseconds = proxy["timeout_milliseconds"].value_or(settings.timeout_milliseconds);
                settings.host = proxy["host"].value_or(settings.host);
                settings.port = static_cast<unsigned short>(proxy["port"].value_or(settings.port));
                settings.log_on = proxy["log_on"].value_or(settings.log_on);
                settings.log_file_name = proxy["log_file_name"].value_or(settings.log_file_name);
                settings.log_file_size_bytes = proxy["log_file_size_bytes"].value_or(settings.log_file_size_bytes);
                settings.max_bandwidth_per_sec = proxy["max_bandwidth_per_sec"].value_or(settings.max_bandwidth_per_sec);
            }
            if(!validate())
            {
                std::cerr << "Loaded settings are invalid, using default values" << std::endl;
                settings = Proxy_Settings{};
            }
            else
            {
                std::cout << "Configuration successfully loaded from " << filename << std::endl;
            }
        }
        else
        {
            std::cout << "Configuration file not found, creating new: " << filename << std::endl;
            toml::table config;
            config.insert_or_assign("proxy", 
            toml::table
            {
                {"max_connections", settings.max_connections},
                {"timeout_milliseconds", settings.timeout_milliseconds},
                {"host", settings.host},
                {"port", settings.port},
                {"log_on", settings.log_on},
                {"log_file_name", settings.log_file_name},
                {"log_file_size_bytes", settings.log_file_size_bytes},
                {"max_bandwidth_per_sec", settings.max_bandwidth_per_sec}
            });
            std::ofstream out_file(filename);
            out_file << config;
            out_file.close();
            std::cout << "Configuration saved to " << filename << std::endl;
        }
    }
    catch(const toml::parse_error& err)
    {
        std::cerr << "TOML parsing error: " << err.what() << std::endl;
        std::cerr << "Using default settings" << std::endl;
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Error working with configuration: " << ex.what() << std::endl;
        std::cerr << "Using default settings" << std::endl;
    }
}