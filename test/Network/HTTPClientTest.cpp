// Copyright (c) 2013-2020 mogemimi. Distributed under the MIT license.

#include "Executor.hpp"
#include "Pomdog/Application/GameClock.hpp"
#include "Pomdog/Network/ArrayView.hpp"
#include "Pomdog/Network/HTTPMethod.hpp"
#include "Pomdog/Network/HTTPClient.hpp"
#include "Pomdog/Network/HTTPRequest.hpp"
#include "Pomdog/Network/HTTPResponse.hpp"
#include "Pomdog/Network/IOService.hpp"
#include "Pomdog/Utility/Errors.hpp"
#include "Pomdog/Utility/StringHelper.hpp"
#include "catch.hpp"
#include <cstring>
#include <sstream>
#include <unordered_map>
#include <thread>

using namespace Pomdog;

TEST_CASE("Transfer-Encoding has a 'chunked' operation.", "[Network]")
{
    Executor executor;

    auto request = HTTPRequest::Create(HTTPMethod::Get, "https://www.google.com");
    request->AddHeader("User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X x.y; rv:42.0) Gecko/20100101 Firefox/42.0");
    auto conn = request->OnCompleted.Connect([&](std::unique_ptr<HTTPResponse>&& response, const std::shared_ptr<Error>& err) {
        if (err != nullptr) {
            WARN("http connection error: " + err->ToString());
            executor.ExitLoop();
            return;
        }
        REQUIRE(response != nullptr);
        REQUIRE(response->Protocol == "HTTP/1.1");
        REQUIRE(response->Status == "200 OK");
        REQUIRE(response->StatusCode == 200);

        REQUIRE(!response->Body.empty());
        std::string_view text{reinterpret_cast<const char*>(response->Body.data()), response->Body.size()};

        REQUIRE(StringHelper::HasPrefix(text,
            "<!doctype html><html itemscope=\"\" itemtype=\"http://schema.org/WebPage\" lang=\""));
        REQUIRE(text.find("<head><meta charset=\"UTF-8\"><meta content=\"origin\" name=\"referrer\">"));
        REQUIRE(StringHelper::HasSuffix(text, "</script></body></html>\r\n"));

        executor.ExitLoop();
    });

    auto client = HTTPClient{executor.GetService()};
    auto err = client.Do(request);
    if (err != nullptr) {
        WARN("http connection error: " + err->ToString());
        return;
    }

    executor.RunLoop();
}

TEST_CASE("HTTPClient Get", "[Network]")
{
    Executor executor;

    auto request = HTTPRequest::Create(HTTPMethod::Get, "http://httpbin.org/get");
    auto conn = request->OnCompleted.Connect([&](std::unique_ptr<HTTPResponse>&& response, const std::shared_ptr<Error>& err) {
        if (err != nullptr) {
            WARN("http connection error: " + err->ToString());
            executor.ExitLoop();
            return;
        }
        REQUIRE(response != nullptr);
        REQUIRE(response->Protocol == "HTTP/1.1");
        REQUIRE(response->Status == "200 OK");
        REQUIRE(response->StatusCode == 200);

        REQUIRE(response->Header.size() >= 7);
        REQUIRE(response->Header[0].first == "Date");
        REQUIRE(response->Header[1].first == "Content-Type");
        REQUIRE(response->Header[1].second == "application/json");
        REQUIRE(response->Header[2].first == "Content-Length");
        REQUIRE(response->Header[2].second == std::to_string(response->ContentLength));
        REQUIRE(response->Header[3].first == "Connection");
        REQUIRE(response->Header[3].second == "keep-alive");
        REQUIRE(response->Header[4].first == "Server");
        REQUIRE(response->Header[5].first == "Access-Control-Allow-Origin");
        REQUIRE(response->Header[5].second == "*");
        REQUIRE(response->Header[6].first == "Access-Control-Allow-Credentials");
        REQUIRE(response->Header[6].second == "true");

        REQUIRE(response->ContentLength == response->Body.size());
        REQUIRE(!response->Body.empty());
        std::string_view text{reinterpret_cast<const char*>(response->Body.data()), response->Body.size()};

        REQUIRE(StringHelper::HasPrefix(text,
            "{\n"
            "  \"args\": {}, \n"
            "  \"headers\": {\n"
            "    \"Host\": \"httpbin.org\", \n"));
        REQUIRE(StringHelper::HasSuffix(text,
            "  \"url\": \"http://httpbin.org/get\"\n"
            "}\n"));

        executor.ExitLoop();
    });

    auto client = HTTPClient{executor.GetService()};
    auto err = client.Do(request);
    if (err != nullptr) {
        WARN("http connection error: " + err->ToString());
        return;
    }

    executor.RunLoop();
}

TEST_CASE("HTTPClient Post", "[Network]")
{
    Executor executor;

    auto request = HTTPRequest::Create(HTTPMethod::Post, "http://httpbin.org/post");
    auto conn = request->OnCompleted.Connect([&](std::unique_ptr<HTTPResponse>&& response, const std::shared_ptr<Error>& err) {
        if (err != nullptr) {
            WARN("http connection error: " + err->ToString());
            executor.ExitLoop();
            return;
        }
        REQUIRE(response != nullptr);
        REQUIRE(response->Protocol == "HTTP/1.1");
        REQUIRE(response->Status == "200 OK");
        REQUIRE(response->StatusCode == 200);

        REQUIRE(response->Header.size() >= 7);
        REQUIRE(response->Header[0].first == "Date");
        REQUIRE(response->Header[1].first == "Content-Type");
        REQUIRE(response->Header[1].second == "application/json");
        REQUIRE(response->Header[2].first == "Content-Length");
        REQUIRE(response->Header[2].second == std::to_string(response->ContentLength));
        REQUIRE(response->Header[3].first == "Connection");
        REQUIRE(response->Header[3].second == "keep-alive");
        REQUIRE(response->Header[4].first == "Server");
        REQUIRE(response->Header[5].first == "Access-Control-Allow-Origin");
        REQUIRE(response->Header[5].second == "*");
        REQUIRE(response->Header[6].first == "Access-Control-Allow-Credentials");
        REQUIRE(response->Header[6].second == "true");

        REQUIRE(response->ContentLength == response->Body.size());
        REQUIRE(!response->Body.empty());
        std::string_view text{reinterpret_cast<const char*>(response->Body.data()), response->Body.size()};

        REQUIRE(StringHelper::HasPrefix(text,
            "{\n"
            "  \"args\": {}, \n"
            "  \"data\": \"hello, hi\", \n"
            "  \"files\": {}, \n"
            "  \"form\": {}, \n"
            "  \"headers\": {\n"
            "    \"Content-Length\": \"9\", \n"
            "    \"Host\": \"httpbin.org\", \n"));
        REQUIRE(std::string_view::npos != text.find(
            "  }, \n"
            "  \"json\": null, \n"));
        REQUIRE(StringHelper::HasSuffix(text,
            "  \"url\": \"http://httpbin.org/post\"\n"
            "}\n"));

        executor.ExitLoop();
    });

    constexpr auto buf = "hello, hi";
    request->Body.resize(std::strlen(buf));
    std::memcpy(request->Body.data(), buf, request->Body.size());

    auto client = HTTPClient{executor.GetService()};
    auto err = client.Do(request);
    if (err != nullptr) {
        WARN("http connection error: " + err->ToString());
        return;
    }

    executor.RunLoop();
}

TEST_CASE("HTTPClient Get Secure", "[Network]")
{
    Executor executor;

    auto request = HTTPRequest::Create(HTTPMethod::Get, "https://httpbin.org/get");
    auto conn = request->OnCompleted.Connect([&](std::unique_ptr<HTTPResponse>&& response, const std::shared_ptr<Error>& err) {
        if (err != nullptr) {
            WARN("http connection error: " + err->ToString());
            executor.ExitLoop();
            return;
        }
        REQUIRE(response != nullptr);
        REQUIRE(response->Protocol == "HTTP/1.1");
        REQUIRE(response->Status == "200 OK");
        REQUIRE(response->StatusCode == 200);

        REQUIRE(response->Header.size() >= 7);
        REQUIRE(response->Header[0].first == "Date");
        REQUIRE(response->Header[1].first == "Content-Type");
        REQUIRE(response->Header[1].second == "application/json");
        REQUIRE(response->Header[2].first == "Content-Length");
        REQUIRE(response->Header[2].second == std::to_string(response->ContentLength));
        REQUIRE(response->Header[3].first == "Connection");
        REQUIRE(response->Header[3].second == "keep-alive");
        REQUIRE(response->Header[4].first == "Server");
        REQUIRE(response->Header[5].first == "Access-Control-Allow-Origin");
        REQUIRE(response->Header[5].second == "*");
        REQUIRE(response->Header[6].first == "Access-Control-Allow-Credentials");
        REQUIRE(response->Header[6].second == "true");

        REQUIRE(response->ContentLength == response->Body.size());
        REQUIRE(!response->Body.empty());
        std::string_view text{reinterpret_cast<const char*>(response->Body.data()), response->Body.size()};

        REQUIRE(StringHelper::HasPrefix(text,
            "{\n"
            "  \"args\": {}, \n"
            "  \"headers\": {\n"
            "    \"Host\": \"httpbin.org\", \n"));
        REQUIRE(StringHelper::HasSuffix(text,
            "  \"url\": \"https://httpbin.org/get\"\n"
            "}\n"));

        executor.ExitLoop();
    });

    auto client = HTTPClient{executor.GetService()};
    auto err = client.Do(request);
    if (err != nullptr) {
        WARN("http connection error: " + err->ToString());
        return;
    }

    executor.RunLoop();
}

TEST_CASE("HTTPClient Post Secure", "[Network]")
{
    Executor executor;

    auto request = HTTPRequest::Create(HTTPMethod::Post, "https://httpbin.org/post");

    constexpr auto buf = "hello, hi";
    request->Body.resize(std::strlen(buf));
    std::memcpy(request->Body.data(), buf, request->Body.size());

    auto conn = request->OnCompleted.Connect([&](std::unique_ptr<HTTPResponse>&& response, const std::shared_ptr<Error>& err) {
        if (err != nullptr) {
            WARN("http connection error: " + err->ToString());
            executor.ExitLoop();
            return;
        }
        REQUIRE(response != nullptr);
        REQUIRE(response->Protocol == "HTTP/1.1");
        REQUIRE(response->Status == "200 OK");
        REQUIRE(response->StatusCode == 200);

        REQUIRE(response->Header.size() >= 7);
        REQUIRE(response->Header[0].first == "Date");
        REQUIRE(response->Header[1].first == "Content-Type");
        REQUIRE(response->Header[1].second == "application/json");
        REQUIRE(response->Header[2].first == "Content-Length");
        REQUIRE(response->Header[2].second == std::to_string(response->ContentLength));
        REQUIRE(response->Header[3].first == "Connection");
        REQUIRE(response->Header[3].second == "keep-alive");
        REQUIRE(response->Header[4].first == "Server");
        REQUIRE(response->Header[5].first == "Access-Control-Allow-Origin");
        REQUIRE(response->Header[5].second == "*");
        REQUIRE(response->Header[6].first == "Access-Control-Allow-Credentials");
        REQUIRE(response->Header[6].second == "true");

        REQUIRE(response->ContentLength == response->Body.size());
        REQUIRE(!response->Body.empty());
        std::string_view text{reinterpret_cast<const char*>(response->Body.data()), response->Body.size()};

        REQUIRE(StringHelper::HasPrefix(text,
            "{\n"
            "  \"args\": {}, \n"
            "  \"data\": \"hello, hi\", \n"
            "  \"files\": {}, \n"
            "  \"form\": {}, \n"
            "  \"headers\": {\n"
            "    \"Content-Length\": \"9\", \n"
            "    \"Host\": \"httpbin.org\", \n"));
        REQUIRE(std::string_view::npos != text.find(
            "  }, \n"
            "  \"json\": null, \n"));
        REQUIRE(StringHelper::HasSuffix(text,
            "  \"url\": \"https://httpbin.org/post\"\n"
            "}\n"));

        executor.ExitLoop();
    });

    auto client = HTTPClient{executor.GetService()};
    auto err = client.Do(request);
    if (err != nullptr) {
        WARN("http connection error: " + err->ToString());
        return;
    }

    executor.RunLoop();
}

TEST_CASE("HTTPClient::Get", "[Network]")
{
    Executor executor;

    auto callback = [&](std::unique_ptr<HTTPResponse>&& response, const std::shared_ptr<Error>& err) {
        if (err != nullptr) {
            WARN("http connection error: " + err->ToString());
            executor.ExitLoop();
            return;
        }
        REQUIRE(response != nullptr);
        REQUIRE(response->Protocol == "HTTP/1.1");
        REQUIRE(response->Status == "200 OK");
        REQUIRE(response->StatusCode == 200);

        REQUIRE(response->Header.size() >= 7);
        REQUIRE(response->Header[0].first == "Date");
        REQUIRE(response->Header[1].first == "Content-Type");
        REQUIRE(response->Header[1].second == "application/json");
        REQUIRE(response->Header[2].first == "Content-Length");
        REQUIRE(response->Header[2].second == std::to_string(response->ContentLength));
        REQUIRE(response->Header[3].first == "Connection");
        REQUIRE(response->Header[3].second == "keep-alive");
        REQUIRE(response->Header[4].first == "Server");
        REQUIRE(response->Header[5].first == "Access-Control-Allow-Origin");
        REQUIRE(response->Header[5].second == "*");
        REQUIRE(response->Header[6].first == "Access-Control-Allow-Credentials");
        REQUIRE(response->Header[6].second == "true");

        REQUIRE(response->ContentLength == response->Body.size());
        REQUIRE(!response->Body.empty());
        std::string_view text{reinterpret_cast<const char*>(response->Body.data()), response->Body.size()};

        REQUIRE(StringHelper::HasPrefix(text,
            "{\n"
            "  \"args\": {}, \n"
            "  \"headers\": {\n"
            "    \"Host\": \"httpbin.org\", \n"));
        REQUIRE(StringHelper::HasSuffix(text,
            "  \"url\": \"https://httpbin.org/get\"\n"
            "}\n"));

        executor.ExitLoop();
    };

    auto client = HTTPClient{executor.GetService()};
    auto[conn, err] = client.Get("https://httpbin.org/get", std::move(callback));
    REQUIRE(err == nullptr);
    REQUIRE(conn.IsConnected());

    executor.RunLoop();
}

TEST_CASE("HTTPClient::Post", "[Network]")
{
    Executor executor;

    auto callback = [&](std::unique_ptr<HTTPResponse>&& response, const std::shared_ptr<Error>& err) {
        if (err != nullptr) {
            WARN("http connection error: " + err->ToString());
            executor.ExitLoop();
            return;
        }
        REQUIRE(response != nullptr);
        REQUIRE(response->Protocol == "HTTP/1.1");
        REQUIRE(response->Status == "200 OK");
        REQUIRE(response->StatusCode == 200);

        REQUIRE(response->Header.size() >= 7);
        REQUIRE(response->Header[0].first == "Date");
        REQUIRE(response->Header[1].first == "Content-Type");
        REQUIRE(response->Header[1].second == "application/json");
        REQUIRE(response->Header[2].first == "Content-Length");
        REQUIRE(response->Header[2].second == std::to_string(response->ContentLength));
        REQUIRE(response->Header[3].first == "Connection");
        REQUIRE(response->Header[3].second == "keep-alive");
        REQUIRE(response->Header[4].first == "Server");
        REQUIRE(response->Header[5].first == "Access-Control-Allow-Origin");
        REQUIRE(response->Header[5].second == "*");
        REQUIRE(response->Header[6].first == "Access-Control-Allow-Credentials");
        REQUIRE(response->Header[6].second == "true");

        REQUIRE(response->ContentLength == response->Body.size());
        REQUIRE(!response->Body.empty());
        std::string_view text{reinterpret_cast<const char*>(response->Body.data()), response->Body.size()};

        REQUIRE(StringHelper::HasPrefix(text,
            "{\n"
            "  \"args\": {}, \n"
            "  \"data\": \"{\\\"answer\\\":42, \\\"text\\\": \\\"hello\\\"}\", \n"
            "  \"files\": {}, \n"
            "  \"form\": {}, \n"
            "  \"headers\": {\n"
            "    \"Content-Length\": \"30\", \n"
            "    \"Content-Type\": \"application/json\", \n"
            "    \"Host\": \"httpbin.org\", \n"));
        REQUIRE(std::string_view::npos != text.find(
            "  }, \n"
            "  \"json\": {\n"
            "    \"answer\": 42, \n"
            "    \"text\": \"hello\"\n"
            "  }, \n"));
        REQUIRE(StringHelper::HasSuffix(text,
            "  \"url\": \"https://httpbin.org/post\"\n"
            "}\n"));

        executor.ExitLoop();
    };

    std::string text = R"({"answer":42, "text": "hello"})";
    std::vector<char> buffer;
    buffer.resize(text.size());
    std::memcpy(buffer.data(), text.data(), buffer.size());

    auto client = HTTPClient{executor.GetService()};
    auto[conn, err] = client.Post("https://httpbin.org/post", "application/json", std::move(buffer), std::move(callback));
    REQUIRE(err == nullptr);
    REQUIRE(conn.IsConnected());

    executor.RunLoop();
}

TEST_CASE("multiple connection", "[Network]")
{
    Executor executor;

    auto client = HTTPClient{executor.GetService()};

    auto callback1 = [&](std::unique_ptr<HTTPResponse>&& response, const std::shared_ptr<Error>& err) {
        if (err != nullptr) {
            WARN("http connection error: " + err->ToString());
            executor.ExitLoop();
            return;
        }
        REQUIRE(response != nullptr);
        REQUIRE(response->Protocol == "HTTP/1.1");
        REQUIRE(response->Status == "200 OK");
        REQUIRE(response->StatusCode == 200);

        auto callback2 = [&](std::unique_ptr<HTTPResponse>&& response, const std::shared_ptr<Error>& err) {
            if (err != nullptr) {
                WARN("http connection error");
                executor.ExitLoop();
                return;
            }
            REQUIRE(response != nullptr);
            REQUIRE(response->Protocol == "HTTP/1.1");
            REQUIRE(response->Status == "200 OK");
            REQUIRE(response->StatusCode == 200);
            executor.ExitLoop();
        };

        auto[conn2, err2] = client.Get("https://httpbin.org/get", std::move(callback2));
        REQUIRE(err2 == nullptr);
        REQUIRE(conn2.IsConnected());
    };

    auto[conn, err] = client.Get("https://httpbin.org/get", std::move(callback1));
    REQUIRE(err == nullptr);
    REQUIRE(conn.IsConnected());

    executor.RunLoop();
}

TEST_CASE("Cancel HTTP Request", "[Network]")
{
    Executor executor;

    auto request = HTTPRequest::Create(HTTPMethod::Get, "http://httpbin.org/get");
    auto conn = request->OnCompleted.Connect([&](std::unique_ptr<HTTPResponse>&& response, const std::shared_ptr<Error>& err) {
        REQUIRE(err != nullptr);
        REQUIRE(response == nullptr);
        executor.ExitLoop();
    });

    constexpr auto buf = "hello, hi";
    request->Body.resize(std::strlen(buf));
    std::memcpy(request->Body.data(), buf, request->Body.size());

    auto client = HTTPClient{executor.GetService()};
    auto err = client.Do(request);
    if (err != nullptr) {
        WARN("http connection error: " + err->ToString());
        return;
    }

    err = client.CancelRequest(request);
    REQUIRE(err == nullptr);

    executor.RunLoop();
}
