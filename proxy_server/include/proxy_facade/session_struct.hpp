#pragma once
#include <string>
#include <cstdint>
#include <arpa/inet.h>

// данная структура нужна на будущее, если в информации о сессии будет что то ещё

struct Session
{
    explicit Session(std::string ip)
    {
        in_addr addr;
        bool ok = (inet_pton(AF_INET, ip.c_str(), &addr) == 1);
        ip_ = ok ? addr.s_addr : 0;
    }
    uint32_t ip_;
};