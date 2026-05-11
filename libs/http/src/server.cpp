#include <obz/http/server.hpp>

#include <obz/transport/tcp_listener.hpp>
#include <obz/transport/tcp_socket.hpp>

#include <charconv>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

namespace obz::http {

namespace {

void validate_target(const std::string& target) {
    if (target.empty() || target.front() != '/') {
        throw std::invalid_argument("HTTP route target must start with '/'");
    }
}

std::string_view trim_ascii(std::string_view value) {
    while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) {
        value.remove_prefix(1);
    }

    while (!value.empty() && (value.back() == ' ' || value.back() == '\t')) {
        value.remove_suffix(1);
    }

    return value;
}

std::size_t parse_content_length_value(std::string_view value) {
    value = trim_ascii(value);

    std::size_t result{};
    const auto* begin = value.data();
    const auto* end = value.data() + value.size();
    const auto parsed = std::from_chars(begin, end, result);

    if (parsed.ec != std::errc{} || parsed.ptr != end) {
        throw std::invalid_argument("invalid HTTP Content-Length header");
    }

    return result;
}

std::string read_request_text(transport::tcp_socket& socket) {
    std::string text;
    std::size_t content_length = 0;
    std::size_t header_end = std::string::npos;

    while (true) {
        if (header_end != std::string::npos && text.size() >= header_end + 4 + content_length) {
            return text;
        }

        const auto chunk = socket.receive(4096);

        if (chunk.empty()) {
            if (text.empty()) {
                throw std::runtime_error("HTTP connection closed before request");
            }

            return text;
        }

        text.append(reinterpret_cast<const char*>(chunk.data()), chunk.size());

        if (header_end == std::string::npos) {
            header_end = text.find("\r\n\r\n");

            if (header_end != std::string::npos) {
                const auto headers = text.substr(0, header_end);
                const auto header_name = std::string{"Content-Length:"};
                const auto length_start = headers.find(header_name);

                if (length_start != std::string::npos) {
                    const auto value_start = length_start + header_name.size();
                    const auto value_end = headers.find("\r\n", value_start);
                    const auto value = headers.substr(value_start, value_end - value_start);
                    content_length = parse_content_length_value(value);
                }
            }
        }
    }
}

void write_all(transport::tcp_socket& socket, std::string_view text) {
    std::size_t bytes_written = 0;

    while (bytes_written < text.size()) {
        const auto remaining = std::as_bytes(std::span{text.data() + bytes_written, text.size() - bytes_written});
        const auto sent = socket.send(remaining);

        if (sent == 0) {
            throw std::runtime_error("HTTP socket send wrote zero bytes");
        }

        bytes_written += sent;
    }
}

void handle_connection(transport::tcp_socket socket, const server& app) {
    try {
        const auto request_text = read_request_text(socket);
        const auto parsed_request = parse_request(request_text);
        write_all(socket, serialize_response(app.handle(parsed_request)));
    } catch (const std::invalid_argument& error) {
        write_all(socket, serialize_response(response::bad_request(error.what())));
    }
}

} // namespace

void server::route(method request_method, std::string target, handler route_handler) {
    if (request_method == method::unknown) {
        throw std::invalid_argument("HTTP route method must be known");
    }

    validate_target(target);

    if (!route_handler) {
        throw std::invalid_argument("HTTP route handler must be callable");
    }

    const route_key key{request_method, std::move(target)};
    const auto [_, inserted] = routes_.emplace(key, std::move(route_handler));

    if (!inserted) {
        throw std::invalid_argument("HTTP route already exists");
    }
}

void server::get(std::string target, handler route_handler) {
    route(method::get, std::move(target), std::move(route_handler));
}

void server::post(std::string target, handler route_handler) {
    route(method::post, std::move(target), std::move(route_handler));
}

void server::put(std::string target, handler route_handler) {
    route(method::put, std::move(target), std::move(route_handler));
}

void server::delete_(std::string target, handler route_handler) {
    route(method::delete_, std::move(target), std::move(route_handler));
}

void server::patch(std::string target, handler route_handler) {
    route(method::patch, std::move(target), std::move(route_handler));
}

response server::handle(const request& value) const {
    const route_key key{value.request_method(), value.target()};
    const auto found = routes_.find(key);

    if (found == routes_.end()) {
        return response::not_found();
    }

    return found->second(value);
}

void server::listen(const transport::endpoint& local_endpoint, int backlog) const {
    transport::tcp_listener listener;
    listener.listen(local_endpoint, backlog);

    while (true) {
        handle_connection(listener.accept(), *this);
    }
}

void server::listen_once(const transport::endpoint& local_endpoint, int backlog) const {
    transport::tcp_listener listener;
    listener.listen(local_endpoint, backlog);
    handle_connection(listener.accept(), *this);
}

} // namespace obz::http
