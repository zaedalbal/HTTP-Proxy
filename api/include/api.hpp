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
        AuthenticationRequest,

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
        bool ProxyStatus;
        unsigned short id;
        CommandName Command;
        bool isChunckedResponse;
        uint32_t data_size;
        std::unique_ptr<char[]> data;
        Response(uint32_t size = 0) : data_size(size), data(size ? std::make_unique<char[]>(size) : nullptr){}
    };

    struct ResponseHeader
    {
        MessageType Type = api::MessageType::Response;
        bool ProxyStatus;
        unsigned short id;
        CommandName Command;
        bool isChunckedResponse;
        uint32_t data_size;
    };
}