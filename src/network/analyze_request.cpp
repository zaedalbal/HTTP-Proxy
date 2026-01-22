#include "network/analyze_request.hpp"
#include "globals/globals.hpp"

HttpHandler::HandlerResult HttpHandler::analyze_request(const boost::beast::http::request<boost::beast::http::string_body>& req)
{
    HttpHandler::HandlerResult result;
    result.is_connect = (req.method() == boost::beast::http::verb::connect);
    std::string_view target = req.target();
    if(result.is_connect)
    {
        auto pos = target.find(':');
        if (pos != std::string_view::npos)
        {
            result.host = std::string(target.substr(0, pos));
            result.port = std::string(target.substr(pos + 1));
        }
        else
        {
            result.host = std::string(target);
            result.port = "443";
        }
    }
    else
    {
        auto host_hdr = req.find(boost::beast::http::field::host);
        std::string_view host_value = host_hdr->value();
        auto pos = host_value.find(':');
        if (pos != std::string_view::npos)
        {
            result.host = std::string(host_value.substr(0, pos));
            result.port = std::string(host_value.substr(pos + 1));
        }
        else
        {
            result.host = std::string(host_value);
            result.port = "80";
        }
    }
    result.is_blacklisted = BLACKLISTED_HOSTS.count(result.host);
    return result;
}