#pragma once

#include <obz/http/request.hpp>
#include <obz/http/response.hpp>
#include <obz/transport/endpoint.hpp>

#include <functional>
#include <map>
#include <string>

namespace obz::http {

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

private:
    using route_key = std::pair<method, std::string>;

    std::map<route_key, handler> routes_;
};

} // namespace obz::http
