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

        Proxy_status,
        Proxy_activeconnections,
        
        Config_get,

        Log_get
    };

    struct SendData
    {
        uint32_t data_size;
        char data[MAX_DATA_SIZE];
    };

    struct Request
    {
        MessageType Type = api::MessageType::Request;
        unsigned short id;
        CommandName Command;
        bool isChunckedRequest;
        SendData data;
    };

    struct Response
    {
        MessageType Type = api::MessageType::Response;
        bool ProxyStatus;
        unsigned short id;
        CommandName Command;
        bool isChunckedResponse;
        SendData data;
    };
}