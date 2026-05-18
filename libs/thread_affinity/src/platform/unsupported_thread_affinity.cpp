#include <obz/thread_affinity.hpp>

#include <system_error>

namespace obz {

bool thread_affinity_supported() noexcept {
    return false;
}

void pin_current_thread_to_cpu(cpu_index) {
    throw std::system_error(
        std::make_error_code(std::errc::not_supported),
        "thread affinity is not supported on this platform");
}

} // namespace obz
