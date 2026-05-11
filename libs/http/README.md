# obz::http

Small HTTP/1.1 request, response, and blocking server helpers.

This library solves one narrow problem: route simple HTTP requests to C++ handlers and serialize clear HTTP responses on top of `obz::transport`.

---

## Features

- HTTP method enum with string conversion
- Request type with method, target, version, headers, and body
- Response type with status, headers, and body
- Exact method/path route matching
- Convenience route methods such as `get` and `post`
- Blocking TCP server built on `obz::transport`
- One request per connection
- `Content-Length` request bodies

---

## Usage

```cpp
#include <obz/http.hpp>

obz::http::server app;

app.get("/health", [](const obz::http::request&) {
    return obz::http::response::text(200, "ok");
});

app.listen({"127.0.0.1", 8080});
```

---

## Basic Example

```cpp
#include <obz/http.hpp>

#include <iostream>

int main() {
    obz::http::server app;

    app.get("/hello", [](const obz::http::request&) {
        return obz::http::response::text(200, "hello");
    });

    std::cout << "listening on http://127.0.0.1:8080/hello\n";
    app.listen({"127.0.0.1", 8080});
}
```

---

## API

### method

```cpp
enum class method {
    get,
    post,
    put,
    delete_,
    patch,
    unknown,
};
```

Represents the supported HTTP request methods.

`delete_` uses a trailing underscore because `delete` is a C++ keyword.

---

### request

```cpp
class request {
public:
    request();
    request(method request_method, std::string target);

    method request_method() const;
    const std::string& target() const;
    const std::string& version() const;

    const headers& all_headers() const;
    void set_header(std::string name, std::string value);
    std::optional<std::string_view> header(std::string_view name) const;

    const std::string& body() const;
    void set_body(std::string value);
};
```

The constructor rejects `method::unknown` and targets that do not start with `/`.

---

### parse_request

```cpp
request parse_request(std::string_view text);
```

Parses a complete HTTP request string.

Throws `std::invalid_argument` when the request line, headers, or `Content-Length` body are malformed.

---

### response

```cpp
class response {
public:
    response();
    response(int status_code, std::string reason_phrase);

    static response text(int status_code, std::string body);
    static response not_found();
    static response bad_request(std::string message);

    int status_code() const;
    const std::string& reason_phrase() const;

    const headers& all_headers() const;
    void set_header(std::string name, std::string value);

    const std::string& body() const;
    void set_body(std::string value);
};
```

The constructor rejects status codes outside the HTTP range `100..599`.

`response::text` sets `Content-Type: text/plain; charset=utf-8`.

---

### serialize_response

```cpp
std::string serialize_response(const response& value);
```

Serializes a response as HTTP/1.1 text and adds `Content-Length` when it is not already present.

Responses include `Connection: close`.

---

### server

```cpp
class server {
public:
    using handler = std::function<response(const request&)>;

    void route(method request_method, std::string target, handler route_handler);

    void get(std::string target, handler route_handler);
    void post(std::string target, handler route_handler);
    void put(std::string target, handler route_handler);
    void delete_(std::string target, handler route_handler);
    void patch(std::string target, handler route_handler);

    response handle(const request& value) const;

    void listen(const transport::endpoint& local_endpoint, int backlog = 8) const;
    void listen_once(const transport::endpoint& local_endpoint, int backlog = 8) const;
};
```

`route` rejects unknown methods, invalid targets, empty handlers, and duplicate method/path pairs.

`handle` returns `404 Not Found` when no route matches.

`listen` accepts and handles connections forever. `listen_once` accepts one connection and returns, which is useful for tests and small examples.

---

## Behaviour Summary

| Situation | Behaviour |
|-----------|-----------|
| matching route | calls the registered handler |
| missing route | returns `404 Not Found` |
| malformed request received by server | sends `400 Bad Request` |
| duplicate route registration | throws `std::invalid_argument` |
| invalid route target | throws `std::invalid_argument` |
| socket failure | propagates `std::system_error` from `obz::transport` |

This first version handles one HTTP request per TCP connection and then closes the connection.

---

## Design Notes

The library is named `http` rather than `http_server` so request, response, method, parser, serializer, server, and a future client can share one protocol namespace.

The server uses exact route matching. A route is identified by `(method, target)`, such as `(GET, "/health")`.

`handle` is separate from `listen` so route dispatch can be tested without opening sockets.

The server depends on `obz::transport` for TCP ownership and byte movement. HTTP parsing and response serialization remain in this library.

---

## Misuse Resistance

- Unknown methods cannot be registered as routes.
- Route targets must start with `/`.
- Duplicate routes are rejected instead of silently replacing handlers.
- Empty handlers are rejected at registration time.
- `Content-Length` is generated automatically for responses unless the caller has already set it.

---

## Implementation Notes

The parser expects a complete HTTP request string with `\r\n` line endings and a `\r\n\r\n` header terminator.

Request bodies are read using `Content-Length`. Chunked transfer encoding is not supported.

The blocking server reads until it has the complete request headers and declared body, parses the request, dispatches to a route, serializes the response, and sends it back over the socket.

---

## When to Use

Use `http` when:

- a small blocking HTTP service is enough
- exact method/path routing is sufficient
- plain HTTP over `obz::transport` is acceptable
- request/response primitives are useful independently of sockets

---

## When Not to Use

Avoid `http` when:

- TLS is required
- asynchronous I/O or event loops are required
- chunked transfer encoding is required
- persistent connections are required
- route parameters or middleware are required
- full HTTP compliance is required

---

## Potential Extensions

Possible extensions, if future use cases justify them:

- `client` for simple HTTP requests
- query string parsing
- route parameters
- middleware hooks
- body size limits
- persistent connections
- chunked transfer encoding

---

## License

Part of the `obz` project.
