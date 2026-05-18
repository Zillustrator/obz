#include <obz/thread_affinity.hpp>

namespace obz {

bool try_pin_current_thread_to_cpu(cpu_index cpu) noexcept {
    try {
        pin_current_thread_to_cpu(cpu);
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace obz
