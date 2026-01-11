#include <gtest/gtest.h>
#include <boost/beast/http.hpp>
#include "network/analyze_request.hpp"

class AnalyzeRequestTest : public ::testing::Test
{
protected:
    HttpHandler handler;
};

// тесты для connect метода
TEST_F(AnalyzeRequestTest, ConnectMethodWithHostAndPort)
{
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::connect, "example.com:8443", 11};
    
    auto result = handler.analyze_request(req);
    
    EXPECT_TRUE(result.is_connect);
    EXPECT_EQ(result.host, "example.com");
    EXPECT_EQ(result.port, "8443");
}

TEST_F(AnalyzeRequestTest, ConnectMethodWithHostOnly)
{
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::connect, "example.com", 11};
    
    auto result = handler.analyze_request(req);
    
    EXPECT_TRUE(result.is_connect);
    EXPECT_EQ(result.host, "example.com");
    EXPECT_EQ(result.port, "443");
}

TEST_F(AnalyzeRequestTest, ConnectMethodWithIPv4AndPort)
{
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::connect, "192.168.1.1:9000", 11};
    
    auto result = handler.analyze_request(req);
    
    EXPECT_TRUE(result.is_connect);
    EXPECT_EQ(result.host, "192.168.1.1");
    EXPECT_EQ(result.port, "9000");
}

TEST_F(AnalyzeRequestTest, ConnectMethodWithStandardHTTPSPort)
{
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::connect, "secure.example.com:443", 11};
    
    auto result = handler.analyze_request(req);
    
    EXPECT_TRUE(result.is_connect);
    EXPECT_EQ(result.host, "secure.example.com");
    EXPECT_EQ(result.port, "443");
}

TEST_F(AnalyzeRequestTest, ConnectMethodWithMultipleColons)
{
    // если в target несколько двоеточий (первое - разделитель)
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::connect, "example.com:8080:extra", 11};
    
    auto result = handler.analyze_request(req);
    
    EXPECT_TRUE(result.is_connect);
    EXPECT_EQ(result.host, "example.com");
    EXPECT_EQ(result.port, "8080:extra");
}

// тесты для get метода (не connect)
TEST_F(AnalyzeRequestTest, GetMethodWithHostAndPort)
{
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get, "/path", 11};
    req.set(boost::beast::http::field::host, "example.com:8080");
    
    auto result = handler.analyze_request(req);
    
    EXPECT_FALSE(result.is_connect);
    EXPECT_EQ(result.host, "example.com");
    EXPECT_EQ(result.port, "8080");
}

TEST_F(AnalyzeRequestTest, GetMethodWithHostOnly)
{
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get, "/api/data", 11};
    req.set(boost::beast::http::field::host, "api.example.com");
    
    auto result = handler.analyze_request(req);
    
    EXPECT_FALSE(result.is_connect);
    EXPECT_EQ(result.host, "api.example.com");
    EXPECT_EQ(result.port, "80");
}

TEST_F(AnalyzeRequestTest, GetMethodWithStandardHTTPPort)
{
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get, "/index.html", 11};
    req.set(boost::beast::http::field::host, "www.example.com:80");
    
    auto result = handler.analyze_request(req);
    
    EXPECT_FALSE(result.is_connect);
    EXPECT_EQ(result.host, "www.example.com");
    EXPECT_EQ(result.port, "80");
}

// тесты для post метода
TEST_F(AnalyzeRequestTest, PostMethodWithHostHeader)
{
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::post, "/submit", 11};
    req.set(boost::beast::http::field::host, "forms.example.com:3000");
    
    auto result = handler.analyze_request(req);
    
    EXPECT_FALSE(result.is_connect);
    EXPECT_EQ(result.host, "forms.example.com");
    EXPECT_EQ(result.port, "3000");
}

// тесты для put метода
TEST_F(AnalyzeRequestTest, PutMethodWithHostOnly)
{
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::put, "/resource/123", 11};
    req.set(boost::beast::http::field::host, "api.example.com");
    
    auto result = handler.analyze_request(req);
    
    EXPECT_FALSE(result.is_connect);
    EXPECT_EQ(result.host, "api.example.com");
    EXPECT_EQ(result.port, "80");
}

// тесты для delete метода
TEST_F(AnalyzeRequestTest, DeleteMethodWithCustomPort)
{
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::delete_, "/item/456", 11};
    req.set(boost::beast::http::field::host, "service.example.com:5000");
    
    auto result = handler.analyze_request(req);
    
    EXPECT_FALSE(result.is_connect);
    EXPECT_EQ(result.host, "service.example.com");
    EXPECT_EQ(result.port, "5000");
}

// граничные случаи
TEST_F(AnalyzeRequestTest, EmptyHost)
{
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::connect, "", 11};
    
    auto result = handler.analyze_request(req);
    
    EXPECT_TRUE(result.is_connect);
    EXPECT_EQ(result.host, "");
    EXPECT_EQ(result.port, "443");
}

TEST_F(AnalyzeRequestTest, HostWithSubdomains)
{
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get, "/", 11};
    req.set(boost::beast::http::field::host, "sub1.sub2.example.com:8888");
    
    auto result = handler.analyze_request(req);
    
    EXPECT_FALSE(result.is_connect);
    EXPECT_EQ(result.host, "sub1.sub2.example.com");
    EXPECT_EQ(result.port, "8888");
}

TEST_F(AnalyzeRequestTest, LocalhostWithPort)
{
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get, "/test", 11};
    req.set(boost::beast::http::field::host, "localhost:3000");
    
    auto result = handler.analyze_request(req);
    
    EXPECT_FALSE(result.is_connect);
    EXPECT_EQ(result.host, "localhost");
    EXPECT_EQ(result.port, "3000");
}

TEST_F(AnalyzeRequestTest, ConnectWithOnlyPort)
{
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::connect, ":8080", 11};
    
    auto result = handler.analyze_request(req);
    
    EXPECT_TRUE(result.is_connect);
    EXPECT_EQ(result.host, "");
    EXPECT_EQ(result.port, "8080");
}

TEST_F(AnalyzeRequestTest, HostHeaderWithMultipleColons)
{
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get, "/", 11};
    req.set(boost::beast::http::field::host, "example.com:8080:extra");
    
    auto result = handler.analyze_request(req);
    
    EXPECT_FALSE(result.is_connect);
    EXPECT_EQ(result.host, "example.com");
    EXPECT_EQ(result.port, "8080:extra");
}

TEST_F(AnalyzeRequestTest, HeadMethod)
{
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::head, "/check", 11};
    req.set(boost::beast::http::field::host, "health.example.com");
    
    auto result = handler.analyze_request(req);
    
    EXPECT_FALSE(result.is_connect);
    EXPECT_EQ(result.host, "health.example.com");
    EXPECT_EQ(result.port, "80");
}

TEST_F(AnalyzeRequestTest, OptionsMethod)
{
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::options, "*", 11};
    req.set(boost::beast::http::field::host, "api.example.com:8443");
    
    auto result = handler.analyze_request(req);
    
    EXPECT_FALSE(result.is_connect);
    EXPECT_EQ(result.host, "api.example.com");
    EXPECT_EQ(result.port, "8443");
}

// граничные случаи для портов
TEST_F(AnalyzeRequestTest, PortWithLeadingZeros)
{
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::connect, "example.com:0080", 11};
    
    auto result = handler.analyze_request(req);
    
    EXPECT_TRUE(result.is_connect);
    EXPECT_EQ(result.host, "example.com");
    EXPECT_EQ(result.port, "0080");
}

TEST_F(AnalyzeRequestTest, HighPortNumber)
{
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::connect, "example.com:65535", 11};
    
    auto result = handler.analyze_request(req);
    
    EXPECT_TRUE(result.is_connect);
    EXPECT_EQ(result.host, "example.com");
    EXPECT_EQ(result.port, "65535");
}