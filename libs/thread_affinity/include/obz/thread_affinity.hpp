#pragma once

#include <cstdint>

namespace obz {

using cpu_index = std::uint32_t;

bool thread_affinity_supported() noexcept;

bool try_pin_current_thread_to_cpu(cpu_index cpu) noexcept;

void pin_current_thread_to_cpu(cpu_index cpu);

} // namespace obz
