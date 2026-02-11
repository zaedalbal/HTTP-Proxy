#pragma once
#include <string>
#include <vector>
#include <cstdint>

#define MAX_DATA_SIZE 16384

namespace api
{
    enum class MessageType
    {
        Request,
        Response,
        Event
    };

    enum class CommandName
    {
        NONE_COMMAND,

        AuthenticationRequest,
        AuthenticationResponseError,
        AuthenticationResponseSuccess,

        Get_proxy_status,
        Get_proxy_sessions,
        Get_proxy_config,
        Get_proxy_log
    };

    struct Request
    {
        MessageType Type = api::MessageType::Request;
        unsigned short id;
        CommandName Command;
        bool isChunckedRequest;
        uint32_t data_size;
        std::unique_ptr<char[]> data;
        Request(uint32_t size = 0) : data_size(size), data(size ? std::make_unique<char[]>(size) : nullptr){}
    };

    struct RequestHeader
    {
        MessageType Type = api::MessageType::Request;
        unsigned short id;
        CommandName Command;
        bool isChunckedRequest;
        uint32_t data_size;
    };

    struct Response
    {
        MessageType Type = api::MessageType::Response;
        bool ProxyStatus = true;
        bool RequestFailed = false;
        unsigned short id;
        CommandName ResponseCommand; // команда для каких то действий в ui
        CommandName RequestCommand; // информация на какую команду ответ
        bool isChunckedResponse;
        uint32_t data_size;
        std::unique_ptr<char[]> data;
        Response(uint32_t size = 0) : data_size(size), data(size ? std::make_unique<char[]>(size) : nullptr){}
    };

    struct ResponseHeader
    {
        MessageType Type = api::MessageType::Response;
        bool ProxyStatus;
        bool RequestFailed;
        unsigned short id;
        CommandName ResponseCommand; // команда для каких то действий в ui
        CommandName RequestCommand; // информация на какую команду ответ
        bool isChunckedResponse;
        uint32_t data_size;
    };

    struct Proxy_Settings // настройки конфига прокси для api
    {
        bool blacklist_on;
        bool log_on;
        bool admin_panel_on;

        unsigned short port;
        unsigned short admin_panel_port;

        int64_t max_connections;
        int64_t timeout_milliseconds;
        int64_t log_file_size_bytes;
        int64_t max_bandwidth_per_sec; 
    };
}