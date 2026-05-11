#include <obz/http.hpp>

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>

TEST_CASE("http converts methods to and from strings") {
    REQUIRE(obz::http::method_from_string("GET") == obz::http::method::get);
    REQUIRE(obz::http::method_from_string("POST") == obz::http::method::post);
    REQUIRE(obz::http::method_from_string("PUT") == obz::http::method::put);
    REQUIRE(obz::http::method_from_string("DELETE") == obz::http::method::delete_);
    REQUIRE(obz::http::method_from_string("PATCH") == obz::http::method::patch);
    REQUIRE(obz::http::method_from_string("TRACE") == obz::http::method::unknown);

    REQUIRE(std::string(obz::http::method_to_string(obz::http::method::delete_)) == "DELETE");
}

TEST_CASE("http parses a simple GET request") {
    const auto request = obz::http::parse_request(
        "GET /health HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n");

    REQUIRE(request.request_method() == obz::http::method::get);
    REQUIRE(request.target() == "/health");
    REQUIRE(request.version() == "HTTP/1.1");
    REQUIRE(std::string(request.header("Host").value()) == "localhost");
    REQUIRE(request.body().empty());
}

TEST_CASE("http parses a request body using content length") {
    const auto request = obz::http::parse_request(
        "POST /items HTTP/1.1\r\n"
        "Content-Length: 7\r\n"
        "\r\n"
        "payload");

    REQUIRE(request.request_method() == obz::http::method::post);
    REQUIRE(request.target() == "/items");
    REQUIRE(request.body() == "payload");
}

TEST_CASE("http rejects malformed requests") {
    REQUIRE_THROWS_AS(obz::http::parse_request("GET /missing-terminator HTTP/1.1\r\n"),
                      std::invalid_argument);

    REQUIRE_THROWS_AS(obz::http::parse_request(
                          "POST /items HTTP/1.1\r\n"
                          "Content-Length: abc\r\n"
                          "\r\n"),
                      std::invalid_argument);

    REQUIRE_THROWS_AS(obz::http::parse_request(
                          "POST /items HTTP/1.1\r\n"
                          "Content-Length: 4\r\n"
                          "\r\n"
                          "abc"),
                      std::invalid_argument);
}

TEST_CASE("http serializes text responses") {
    const auto response = obz::http::response::text(200, "ok");
    const auto text = obz::http::serialize_response(response);

    REQUIRE(text.find("HTTP/1.1 200 OK\r\n") == 0);
    REQUIRE(text.find("Content-Type: text/plain; charset=utf-8\r\n") != std::string::npos);
    REQUIRE(text.find("Content-Length: 2\r\n") != std::string::npos);
    REQUIRE(text.find("Connection: close\r\n") != std::string::npos);
    REQUIRE(text.ends_with("\r\n\r\nok"));
}

TEST_CASE("http server dispatches matching routes") {
    obz::http::server server;
    server.get("/health", [](const obz::http::request&) {
        return obz::http::response::text(200, "healthy");
    });

    const obz::http::request request(obz::http::method::get, "/health");
    const auto response = server.handle(request);

    REQUIRE(response.status_code() == 200);
    REQUIRE(response.body() == "healthy");
}

TEST_CASE("http server returns not found for missing routes") {
    const obz::http::server server;
    const obz::http::request request(obz::http::method::get, "/missing");

    const auto response = server.handle(request);

    REQUIRE(response.status_code() == 404);
    REQUIRE(response.body() == "not found");
}

TEST_CASE("http server rejects ambiguous or invalid routes") {
    obz::http::server server;
    server.get("/items", [](const obz::http::request&) {
        return obz::http::response::text(200, "ok");
    });

    REQUIRE_THROWS_AS(
        server.get("/items", [](const obz::http::request&) {
            return obz::http::response::text(200, "duplicate");
        }),
        std::invalid_argument);

    REQUIRE_THROWS_AS(
        server.get("items", [](const obz::http::request&) {
            return obz::http::response::text(200, "invalid");
        }),
        std::invalid_argument);
}
