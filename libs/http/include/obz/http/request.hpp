#pragma once

#include <obz/http/method.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace obz::http {

using headers = std::unordered_map<std::string, std::string>;

class request {
public:
    request() = default;
    request(method request_method, std::string target);

    method request_method() const;
    const std::string& target() const;

    const std::string& version() const;
    void set_version(std::string value);

    const headers& all_headers() const;
    void set_header(std::string name, std::string value);
    std::optional<std::string_view> header(std::string_view name) const;

    const std::string& body() const;
    void set_body(std::string value);

private:
    method method_{method::unknown};
    std::string target_;
    std::string version_{"HTTP/1.1"};
    headers headers_;
    std::string body_;
};

request parse_request(std::string_view text);

} // namespace obz::http
