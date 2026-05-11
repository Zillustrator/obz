#pragma once

#include <obz/http/request.hpp>

#include <string>
#include <string_view>

namespace obz::http {

class response {
public:
    response() = default;
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

private:
    int status_code_{200};
    std::string reason_phrase_{"OK"};
    headers headers_;
    std::string body_;
};

std::string serialize_response(const response& value);
std::string default_reason_phrase(int status_code);

} // namespace obz::http
