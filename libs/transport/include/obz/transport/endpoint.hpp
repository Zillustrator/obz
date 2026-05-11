#pragma once

#include <cstdint>
#include <string>

namespace obz::transport {

struct endpoint {
    std::string host;
    std::uint16_t port{};
};

} // namespace obz::transport
