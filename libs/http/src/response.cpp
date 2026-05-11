#include <obz/http/response.hpp>

#include <stdexcept>
#include <string>
#include <utility>

namespace obz::http {

response::response(int status_code, std::string reason_phrase)
    : status_code_(status_code),
      reason_phrase_(std::move(reason_phrase)) {
    if (status_code_ < 100 || status_code_ > 599) {
        throw std::invalid_argument("HTTP status code must be in the range 100..599");
    }

    if (reason_phrase_.empty()) {
        reason_phrase_ = default_reason_phrase(status_code_);
    }
}

response response::text(int status_code, std::string body) {
    response result(status_code, default_reason_phrase(status_code));
    result.set_header("Content-Type", "text/plain; charset=utf-8");
    result.set_body(std::move(body));
    return result;
}

response response::not_found() {
    return text(404, "not found");
}

response response::bad_request(std::string message) {
    return text(400, std::move(message));
}

int response::status_code() const {
    return status_code_;
}

const std::string& response::reason_phrase() const {
    return reason_phrase_;
}

const headers& response::all_headers() const {
    return headers_;
}

void response::set_header(std::string name, std::string value) {
    if (name.empty()) {
        throw std::invalid_argument("HTTP header name must not be empty");
    }

    headers_[std::move(name)] = std::move(value);
}

const std::string& response::body() const {
    return body_;
}

void response::set_body(std::string value) {
    body_ = std::move(value);
}

std::string serialize_response(const response& value) {
    std::string result;
    result += "HTTP/1.1 ";
    result += std::to_string(value.status_code());
    result += ' ';
    result += value.reason_phrase();
    result += "\r\n";

    bool has_content_length = false;

    for (const auto& [name, header_value] : value.all_headers()) {
        if (name == "Content-Length") {
            has_content_length = true;
        }

        result += name;
        result += ": ";
        result += header_value;
        result += "\r\n";
    }

    if (!has_content_length) {
        result += "Content-Length: ";
        result += std::to_string(value.body().size());
        result += "\r\n";
    }

    result += "Connection: close\r\n";
    result += "\r\n";
    result += value.body();

    return result;
}

std::string default_reason_phrase(int status_code) {
    switch (status_code) {
    case 200:
        return "OK";
    case 201:
        return "Created";
    case 204:
        return "No Content";
    case 400:
        return "Bad Request";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 500:
        return "Internal Server Error";
    default:
        return "Unknown";
    }
}

} // namespace obz::http
