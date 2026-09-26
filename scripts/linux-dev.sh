#!/usr/bin/env bash
set -euo pipefail

readonly script_directory="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly source_directory="$(cd "${script_directory}/.." && pwd)"
readonly image="obzlib-linux-dev:ubuntu24.04"
readonly build_volume="obzlib-linux-build"

usage() {
    cat <<'EOF'
Usage: scripts/linux-dev.sh <command>

  build-image          Build the Ubuntu development image
  shell                Open an interactive Linux shell
  test-gcc             Build and test with GCC
  test-clang           Build and test with Clang
  test-gcc-sanitize    Build and test with GCC, ASan and UBSan
  test-clang-sanitize  Build and test with Clang, ASan and UBSan
  test-multicast       Build with GCC and run multicast integration tests

Sources are mounted read-only at /workspace/obz; builds persist in /build.
Build the image first. VS Code tasks do this automatically.
EOF
}

run_tests() {
    docker run --rm \
        --mount "type=bind,source=${source_directory},target=/workspace/obz,readonly" \
        --mount "type=volume,source=${build_volume},target=/build" \
        "${image}" bash -c '
set -euo pipefail
compiler="$1"
build_directory="/build/$2"
sanitizers="$3"
mode="$4"
cmake -S /workspace/obz -B "$build_directory" -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER="$compiler" \
    -DOBZ_BUILD_TESTS=ON -DOBZ_BUILD_EXAMPLES=OFF \
    -DOBZ_ENABLE_ASAN_UBSAN="$sanitizers"
cmake --build "$build_directory" --parallel 4
if [[ "$mode" == multicast ]]; then
    timeout 60 "$build_directory/tests/obz_tests" "[.multicast],[.multicast-membership]"
else
    extra_args=()
    if [[ "$sanitizers" == ON ]]; then
        extra_args+=(--exclude-regex "^obz_package_consumer$")
    fi
    ctest --test-dir "$build_directory" --output-on-failure --timeout 60 "${extra_args[@]}"
fi
' obz-linux-tests "$@"
}

case "${1:-}" in
    build-image)
        docker build -t "$image" "${source_directory}/docker/linux-dev"
        ;;
    shell)
        docker run --rm -it \
            --mount "type=bind,source=${source_directory},target=/workspace/obz,readonly" \
            --mount "type=volume,source=${build_volume},target=/build" \
            "$image"
        ;;
    test-gcc) run_tests g++ gcc OFF suite ;;
    test-clang) run_tests clang++ clang OFF suite ;;
    test-gcc-sanitize) run_tests g++ gcc-asan-ubsan ON suite ;;
    test-clang-sanitize) run_tests clang++ clang-asan-ubsan ON suite ;;
    test-multicast) run_tests g++ gcc OFF multicast ;;
    --help|-h) usage ;;
    *) usage >&2; exit 2 ;;
esac
