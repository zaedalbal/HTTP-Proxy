#include "config/proxy_config.hpp"
#include <toml++/toml.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>

Proxy_Config::Proxy_Config()
{
    load_or_create_cfg("proxy_config.toml");
}

bool Proxy_Config::validate() const
{
    bool error_flag = false;
    if(settings.max_connections < 1)
    {
        std::cerr << "Error in config: max_connection must be at least 1" << std::endl;
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
    if(settings.admin_panel_port < 1)
    {
        std::cerr << "Error in config: admin panel port must be greater than 0" << std::endl;
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
    if(settings.blacklisted_hosts_file_name.empty())
    {
        std::cerr << "Error in config: blacklisted_hosts_file_name cannot be empty" << std::endl;
        error_flag = true;
    }
    if(settings.admin_panel_auth_data_file_name.empty())
    {
        std::cerr << "Error in config: admin_panel_auth_data_file_name is empty" << std::endl;
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
                settings.blacklist_on = proxy["blacklist_on"].value_or(settings.blacklist_on);
                settings.blacklisted_hosts_file_name = proxy["blacklisted_hosts_file_name"].value_or(settings.blacklisted_hosts_file_name);
                settings.admin_panel_on = proxy["admin_panel_on"].value_or(settings.admin_panel_on);
                settings.admin_panel_port = proxy["admin_panel_port"].value_or(settings.admin_panel_port);
                settings.admin_panel_auth_data_file_name = proxy["admin_panel_auth_data_file_name"].value_or(settings.admin_panel_auth_data_file_name);
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
                {"max_bandwidth_per_sec", settings.max_bandwidth_per_sec},
                {"blacklist_on", settings.blacklist_on},
                {"blacklisted_hosts_file_name", settings.blacklisted_hosts_file_name},
                {"admin_panel_on", settings.admin_panel_on},
                {"admin_panel_port", settings.admin_panel_port},
                {"admin_panel_auth_data_file_name", settings.admin_panel_auth_data_file_name}
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

std::unordered_set<std::string> Proxy_Config::get_blacklisted_hosts() const
{
    std::unordered_set<std::string> blacklisted_hosts;
    const auto filename = settings.blacklisted_hosts_file_name;
    try
    {
        auto blacklist = toml::parse_file(filename);
        if(blacklist["blacklist"] && blacklist["blacklist"]["hosts"])
        {
            auto hosts_array = blacklist["blacklist"]["hosts"].as_array();
            for(const auto& i : *hosts_array)
            {
                if(i.is_string())
                {
                    blacklisted_hosts.insert(i.value_or(""));
                }
            }
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
    return blacklisted_hosts;
}

// функция для использования в Proxy_Config::find_admin_panel_account_by_login
inline std::vector<uint8_t> hex_to_bytes(const std::string& hex)
{
    std::vector<uint8_t> bytes;
    if (hex.size() % 2 != 0) return bytes; // на всякий случай
    bytes.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2)
    {
        uint8_t byte = static_cast<uint8_t>(std::stoul(hex.substr(i, 2), nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

std::optional<Proxy_Config::Admin_panel_account> 
Proxy_Config::find_admin_panel_account_by_login(const std::string& login, const std::string& filename)
{
    if(!std::filesystem::exists(filename))
        return std::nullopt;
    toml::table table;
    try
    {
        table = toml::parse_file(filename);
    }
    catch(const toml::parse_error& err)
    {
        return std::nullopt;
    }
    if(!table.contains(login))
        return std::nullopt;
    const auto& account_node = table[login];
    Proxy_Config::Admin_panel_account account;
    account.login = login;
    if(auto algo = account_node["algorithm"].value<std::string>(); algo.has_value())
        account.algorithm = *algo;
    else
        return std::nullopt;
    if(auto iterations = account_node["iterations"].value<uint32_t>(); iterations.has_value())
        account.iterations = *iterations;
    else
        return std::nullopt;
    if(auto salt = account_node["salt"].value<std::string>(); salt.has_value())
        account.salt = hex_to_bytes(*salt);
    else
        return std::nullopt;
    if(auto hash = account_node["hash"].value<std::string>(); hash.has_value())
        account.hash = hex_to_bytes(*hash);
    else
        return std::nullopt;

    return account;
}