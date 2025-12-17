#pragma once
#include <boost/beast.hpp>
#include <string>

class HttpHandler
{
    public:
        struct HandlerResult
        {
            bool is_connect;
            std::string host;
            std::string port;
        };
    static HandlerResult analyze_request(const boost::beast::http::request<boost::beast::http::string_body>& req);
};