#include <obz/thread_affinity.hpp>

#include <pthread.h>
#include <sched.h>

#include <stdexcept>
#include <system_error>

namespace obz {

bool thread_affinity_supported() noexcept {
    return true;
}

void pin_current_thread_to_cpu(cpu_index cpu) {
    if (cpu >= static_cast<cpu_index>(CPU_SETSIZE)) {
        throw std::invalid_argument("thread_affinity CPU index exceeds CPU_SETSIZE");
    }

    cpu_set_t cpuset{};
    CPU_ZERO(&cpuset);
    CPU_SET(static_cast<int>(cpu), &cpuset);

    const auto result = ::pthread_setaffinity_np(
        ::pthread_self(),
        sizeof(cpu_set_t),
        &cpuset);

    if (result != 0) {
        throw std::system_error(
            result,
            std::generic_category(),
            "failed to pin current thread to CPU");
    }
}

} // namespace obz
