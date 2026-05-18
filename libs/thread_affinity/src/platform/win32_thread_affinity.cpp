#include <obz/thread_affinity.hpp>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include <limits>
#include <stdexcept>
#include <system_error>

namespace obz {

bool thread_affinity_supported() noexcept {
    return true;
}

void pin_current_thread_to_cpu(cpu_index cpu) {
    constexpr auto mask_bits = std::numeric_limits<DWORD_PTR>::digits;

    if (cpu >= mask_bits) {
        throw std::invalid_argument("thread_affinity CPU index exceeds affinity mask width");
    }

    const auto mask = static_cast<DWORD_PTR>(1) << cpu;
    const auto previous_mask = ::SetThreadAffinityMask(::GetCurrentThread(), mask);

    if (previous_mask == 0) {
        throw std::system_error(
            static_cast<int>(::GetLastError()),
            std::system_category(),
            "failed to pin current thread to CPU");
    }
}

} // namespace obz
