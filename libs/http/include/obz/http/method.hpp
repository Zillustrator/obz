#pragma once

#include <string_view>

namespace obz::http {

enum class method {
    get,
    post,
    put,
    delete_,
    patch,
    unknown,
};

method method_from_string(std::string_view value);
std::string_view method_to_string(method value);

} // namespace obz::http
