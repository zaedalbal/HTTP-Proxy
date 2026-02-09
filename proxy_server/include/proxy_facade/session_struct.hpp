#pragma once
#include <string>

// данная структура нужна на будущее, если в информации о сессии будет что то ещё

struct Session
{
    Session(std::string ip) : ip_(ip){}
    std::string ip_;
};
