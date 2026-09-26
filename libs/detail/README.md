# Shared implementation headers

Internal header-only support for ObzLib components. Applications should link the
public component they use; its `INTERFACE` dependency supplies `obz::detail`
transitively. These headers are installed with the package because public
component headers include them, but are not a separate supported application API.

`obz/detail/cache_alignment.hpp` defines `destructive_interference_size` for
separating frequently written state. It uses the standard library feature-test
macro to select the implementation recommendation, or 64 bytes when unavailable.
The fallback is not a hardware measurement or a universal false-sharing guarantee.

The selected value affects object layout. Use consistent toolchain and target
settings in translation units that share the affected types. The queue READMEs
explain which members are aligned and the resulting compatibility constraints.
