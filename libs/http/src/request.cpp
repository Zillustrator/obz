#include <obz/http/request.hpp>

#include <charconv>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>

namespace obz::http {

namespace {

std::string_view trim_ascii(std::string_view value) {
    while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) {
        value.remove_prefix(1);
    }

    while (!value.empty() && (value.back() == ' ' || value.back() == '\t')) {
        value.remove_suffix(1);
    }

    return value;
}

std::size_t parse_content_length(std::string_view value) {
    std::size_t result{};
    const auto* begin = value.data();
    const auto* end = value.data() + value.size();
    const auto parsed = std::from_chars(begin, end, result);

    if (parsed.ec != std::errc{} || parsed.ptr != end) {
        throw std::invalid_argument("invalid HTTP Content-Length header");
    }

    return result;
}

} // namespace

method method_from_string(std::string_view value) {
    if (value == "GET") return method::get;
    if (value == "POST") return method::post;
    if (value == "PUT") return method::put;
    if (value == "DELETE") return method::delete_;
    if (value == "PATCH") return method::patch;
    return method::unknown;
}

std::string_view method_to_string(method value) {
    switch (value) {
    case method::get:
        return "GET";
    case method::post:
        return "POST";
    case method::put:
        return "PUT";
    case method::delete_:
        return "DELETE";
    case method::patch:
        return "PATCH";
    case method::unknown:
        return "UNKNOWN";
    }

    return "UNKNOWN";
}

request::request(method request_method, std::string target)
    : method_(request_method),
      target_(std::move(target)) {
    if (method_ == method::unknown) {
        throw std::invalid_argument("HTTP request method must be known");
    }

    if (target_.empty() || target_.front() != '/') {
        throw std::invalid_argument("HTTP request target must start with '/'");
    }
}

method request::request_method() const {
    return method_;
}

const std::string& request::target() const {
    return target_;
}

const std::string& request::version() const {
    return version_;
}

void request::set_version(std::string value) {
    if (value.empty()) {
        throw std::invalid_argument("HTTP version must not be empty");
    }

    version_ = std::move(value);
}

const headers& request::all_headers() const {
    return headers_;
}

void request::set_header(std::string name, std::string value) {
    if (name.empty()) {
        throw std::invalid_argument("HTTP header name must not be empty");
    }

    headers_[std::move(name)] = std::move(value);
}

std::optional<std::string_view> request::header(std::string_view name) const {
    for (const auto& current : headers_) {
        if (current.first == name) {
            return current.second;
        }
    }

    return std::nullopt;
}

const std::string& request::body() const {
    return body_;
}

void request::set_body(std::string value) {
    body_ = std::move(value);
}

request parse_request(std::string_view text) {
    const auto header_end = text.find("\r\n\r\n");

    if (header_end == std::string_view::npos) {
        throw std::invalid_argument("HTTP request is missing header terminator");
    }

    const auto start_line_end = text.find("\r\n");

    if (start_line_end == std::string_view::npos || start_line_end > header_end) {
        throw std::invalid_argument("HTTP request is missing request line");
    }

    const auto start_line = text.substr(0, start_line_end);
    const auto method_end = start_line.find(' ');

    if (method_end == std::string_view::npos) {
        throw std::invalid_argument("HTTP request line is missing method");
    }

    const auto target_start = method_end + 1;
    const auto target_end = start_line.find(' ', target_start);

    if (target_end == std::string_view::npos) {
        throw std::invalid_argument("HTTP request line is missing target");
    }

    const auto method_value = method_from_string(start_line.substr(0, method_end));
    const auto target_value = start_line.substr(target_start, target_end - target_start);
    const auto version_value = start_line.substr(target_end + 1);

    request result(method_value, std::string(target_value));
    result.set_version(std::string(version_value));

    std::size_t line_start = start_line_end + 2;
    std::size_t content_length = 0;

    while (line_start < header_end) {
        const auto line_end = text.find("\r\n", line_start);

        if (line_end == std::string_view::npos || line_end > header_end) {
            throw std::invalid_argument("HTTP request contains malformed header line");
        }

        const auto line = text.substr(line_start, line_end - line_start);
        const auto separator = line.find(':');

        if (separator == std::string_view::npos) {
            throw std::invalid_argument("HTTP request header is missing ':'");
        }

        const auto name = trim_ascii(line.substr(0, separator));
        const auto value = trim_ascii(line.substr(separator + 1));

        if (name.empty()) {
            throw std::invalid_argument("HTTP request header name must not be empty");
        }

        result.set_header(std::string(name), std::string(value));

        if (name == "Content-Length") {
            content_length = parse_content_length(value);
        }

        line_start = line_end + 2;
    }

    const auto body_start = header_end + 4;

    if (text.size() - body_start < content_length) {
        throw std::invalid_argument("HTTP request body is shorter than Content-Length");
    }

    result.set_body(std::string(text.substr(body_start, content_length)));
    return result;
}

} // namespace obz::http
