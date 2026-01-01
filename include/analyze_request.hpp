#pragma once
#include <boost/beast.hpp>
#include <string>

class HttpHandler
{
    public:
        struct HandlerResult
        {
            bool is_connect; // проверка на метод CONNECT
            std::string host; // имя хоста из запроса
            std::string port; // порт из запроса
        };
    static HandlerResult analyze_request(const boost::beast::http::request<boost::beast::http::string_body>& req); // анализ запроса
};