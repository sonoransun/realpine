#!/usr/bin/env bash
# Alpine local CI runner.
#
# A single entry point that produces the same reproducible output whether
# invoked locally on a developer machine or from a hosted runner.  Each
# subcommand configures its own out-of-source build directory and logs
# its output to `build-ci/<job>.log` so a failing run leaves something to
# attach to a bug report.
#
# Subcommands:
#   build [gcc|clang] [Release|Debug]
#   test  [gcc|clang] [Release|Debug]
#   sanitize
#   coverage
#   format-check
#   format-fix
#   docker-build
#   cluster-test
#   all
#   help

set -Eeuo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CI_DIR="${ROOT_DIR}/build-ci"
mkdir -p "${CI_DIR}"

# shellcheck source=ci-env.sh
source "${ROOT_DIR}/scripts/ci-env.sh"

log()   { printf '\033[1;34m[ci]\033[0m %s\n' "$*"; }
warn()  { printf '\033[1;33m[ci]\033[0m %s\n' "$*" >&2; }
die()   { printf '\033[1;31m[ci]\033[0m %s\n' "$*" >&2; exit 1; }

NPROC="$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

# Format-check file set — keep in sync with CMake targets that we want
# lint-enforced.  Third-party vendored code under AlpineGui/vendor, _deps,
# and build* directories is deliberately excluded.
format_files () {
    (
        cd "${ROOT_DIR}"
        find base protocols/Alpine transport applcore interfaces \
             AlpineRestBridge AlpineCmdIf AlpineServer AlpineGui \
             test/unit test/integration test/load bench fuzz \
             test/tcp_conn_test test/udp_send_test test/udp_socket_conn \
             test/dtcp_stack test/wifi_hwsim test/auto_thread_test \
             test/sys_thread_test test/map_test test/spawn_process test/dl_test \
             -name '*.h' -o -name '*.cpp' -o -name '*.hpp' 2>/dev/null \
            | grep -v AlpineGui/vendor \
            | grep -v _deps
    )
}

# --- Subcommands ---------------------------------------------------------

cmd_build () {
    local tc="${1:-gcc}"
    local type="${2:-Debug}"
    local cc cxx
    case "${tc}" in
        gcc)   cc="$(ci_gcc_c)";   cxx="$(ci_gcc_cxx)";;
        clang) cc="$(ci_clang_c)"; cxx="$(ci_clang_cxx)";;
        *) die "unknown toolchain '${tc}' (expected gcc or clang)";;
    esac
    [[ -x "${cc}"  ]] || die "${cc} not found — install it or pick a different toolchain"
    [[ -x "${cxx}" ]] || die "${cxx} not found — install it or pick a different toolchain"

    local build="${CI_DIR}/${tc}-${type}"
    log "Configure ${tc}/${type} → ${build}"
    cmake -B "${build}" -S "${ROOT_DIR}" \
        -DCMAKE_BUILD_TYPE="${type}" \
        -DCMAKE_C_COMPILER="${cc}" \
        -DCMAKE_CXX_COMPILER="${cxx}" \
        -DALPINE_WARNINGS_AS_ERRORS=ON \
        > "${CI_DIR}/${tc}-${type}-configure.log" 2>&1

    log "Build ${tc}/${type} (${NPROC} jobs)"
    cmake --build "${build}" -j "${NPROC}" \
        > "${CI_DIR}/${tc}-${type}-build.log" 2>&1 \
        || { tail -40 "${CI_DIR}/${tc}-${type}-build.log"; die "build failed"; }
}

cmd_test () {
    local tc="${1:-gcc}"
    local type="${2:-Debug}"
    local build="${CI_DIR}/${tc}-${type}"
    [[ -d "${build}" ]] || cmd_build "${tc}" "${type}"

    log "Run ctest ${tc}/${type}"
    (
        cd "${build}"
        ctest --output-on-failure --timeout 120
    ) | tee "${CI_DIR}/${tc}-${type}-test.log"
}

cmd_sanitize () {
    local cc cxx
    cc="$(ci_clang_c)"; cxx="$(ci_clang_cxx)"
    [[ -x "${cc}"  ]] || die "clang not found — install clang-18"
    [[ -x "${cxx}" ]] || die "clang++ not found — install clang-18"

    # Clang + libstdc++14 is missing <expected> (and a few other C++23
    # feature test macros), which Alpine's base/Error.h relies on.  Use
    # libc++ instead so the sanitizer job is isolated from any host
    # libstdc++ version skew.
    local build="${CI_DIR}/asan-ubsan"
    log "Configure clang ASan+UBSan (libc++) → ${build}"
    # `-D_LIBCPP_ENABLE_EXPERIMENTAL` is required so libc++18 exposes
    # `<stop_token>` / `<expected>` / `<jthread>` (all shipped as experimental
    # in that release).  Alpine's ThreadUtils is written against `jthread`.
    # `-fno-sanitize=function` drops UBSan's function-type-mismatch check —
    # Alpine's plugin layer dlsym's a `create_DynamicObject` factory and casts
    # it to the expected signature, which is a C-standard-legal cast but UBSan
    # >= 10 flags it conservatively.  `-fno-sanitize=vptr,null` quiets libc++
    # internal null-sentinel sentinels used in empty unordered_map buckets.
    cmake -B "${build}" -S "${ROOT_DIR}" \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_C_COMPILER="${cc}" \
        -DCMAKE_CXX_COMPILER="${cxx}" \
        -DCMAKE_CXX_FLAGS="-stdlib=libc++ -D_LIBCPP_ENABLE_EXPERIMENTAL -fno-sanitize=function,vptr,null" \
        -DCMAKE_C_FLAGS="-fno-sanitize=function,vptr,null" \
        -DCMAKE_EXE_LINKER_FLAGS="-stdlib=libc++ -lc++experimental" \
        -DCMAKE_SHARED_LINKER_FLAGS="-stdlib=libc++ -lc++experimental" \
        -DALPINE_SANITIZER=address,undefined \
        > "${CI_DIR}/asan-ubsan-configure.log" 2>&1

    log "Build (${NPROC} jobs)"
    cmake --build "${build}" -j "${NPROC}" \
        > "${CI_DIR}/asan-ubsan-build.log" 2>&1 \
        || { tail -40 "${CI_DIR}/asan-ubsan-build.log"; die "build failed"; }

    log "Run ctest under ASan+UBSan"
    # `alloc_dealloc_mismatch=0` suppresses libc++ small-string-optimization
    # false positives that fire when a std::string is grown across a libc++
    # internal path.  detect_leaks=0 because LeakSanitizer is too chatty
    # against Catch2's static test registry.
    ASAN_OPTIONS="halt_on_error=1:abort_on_error=1:detect_leaks=0:alloc_dealloc_mismatch=0:strict_string_checks=1" \
    UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1" \
    LSAN_OPTIONS="suppressions=${ROOT_DIR}/test/asan.supp" \
    ctest --test-dir "${build}" --output-on-failure --timeout 180 \
        | tee "${CI_DIR}/asan-ubsan-test.log"
}

cmd_coverage () {
    local cc cxx
    cc="$(ci_gcc_c)"; cxx="$(ci_gcc_cxx)"
    [[ -x "${cc}" && -x "${cxx}" ]] || die "gcc-14 required for coverage"
    command -v lcov >/dev/null   || die "lcov required for coverage"
    command -v genhtml >/dev/null || die "genhtml (lcov package) required for coverage"

    local build="${CI_DIR}/coverage"
    log "Configure coverage build → ${build}"
    cmake -B "${build}" -S "${ROOT_DIR}" \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_C_COMPILER="${cc}" \
        -DCMAKE_CXX_COMPILER="${cxx}" \
        -DALPINE_ENABLE_COVERAGE=ON \
        > "${CI_DIR}/coverage-configure.log" 2>&1

    log "Build coverage (${NPROC} jobs)"
    cmake --build "${build}" -j "${NPROC}" \
        > "${CI_DIR}/coverage-build.log" 2>&1 \
        || { tail -40 "${CI_DIR}/coverage-build.log"; die "build failed"; }

    log "Run ctest under coverage"
    ctest --test-dir "${build}" --output-on-failure --timeout 180 \
        | tee "${CI_DIR}/coverage-test.log"

    log "Collect coverage → ${build}/coverage/"
    ( cd "${build}" && cmake --build . --target coverage ) \
        > "${CI_DIR}/coverage-collect.log" 2>&1 || {
            tail -40 "${CI_DIR}/coverage-collect.log"
            die "coverage collection failed"
        }

    log "Coverage report: ${build}/coverage/index.html"
}

cmd_format_check () {
    local cf
    cf="$(ci_clang_format)"
    [[ -x "${cf}" ]] || die "clang-format-18 not found"

    log "clang-format --dry-run -Werror"
    local failed=0
    while IFS= read -r f; do
        "${cf}" --style=file --dry-run -Werror "${ROOT_DIR}/${f}" 2>> "${CI_DIR}/format-check.log" \
            || failed=1
    done < <(format_files)
    if (( failed != 0 )); then
        tail -60 "${CI_DIR}/format-check.log"
        die "format check failed — run 'scripts/ci.sh format-fix' to auto-fix"
    fi
    log "format check OK"
}

cmd_format_fix () {
    local cf
    cf="$(ci_clang_format)"
    [[ -x "${cf}" ]] || die "clang-format-18 not found"
    log "Applying clang-format to tracked source tree"
    format_files | xargs -I{} -P "${NPROC}" "${cf}" --style=file -i "${ROOT_DIR}/{}"
    log "format fix complete"
}

cmd_docker_build () {
    command -v docker >/dev/null || die "docker not installed"
    log "docker build: Dockerfile (ubuntu)"
    ( cd "${ROOT_DIR}" && \
        docker buildx build --load --platform linux/amd64 \
            -f docker/Dockerfile -t alpine:ci-ubuntu . ) \
        > "${CI_DIR}/docker-ubuntu.log" 2>&1 || {
            tail -40 "${CI_DIR}/docker-ubuntu.log"
            die "ubuntu docker build failed"
        }
    log "docker build: Dockerfile.alpine (musl)"
    ( cd "${ROOT_DIR}" && \
        docker buildx build --load --platform linux/amd64 \
            -f docker/Dockerfile.alpine -t alpine:ci-alpine . ) \
        > "${CI_DIR}/docker-alpine.log" 2>&1 || {
            tail -40 "${CI_DIR}/docker-alpine.log"
            die "alpine docker build failed"
        }
    log "docker builds OK"
}

cmd_cluster_test () {
    command -v docker >/dev/null || die "docker not installed"
    local script="${ROOT_DIR}/test/docker/run_cluster_tests.sh"
    [[ -x "${script}" ]] || die "${script} not found — run Phase C(e) to add it"
    "${script}"
}

cmd_all () {
    local failures=()

    _try () {
        local name="$1"; shift
        log "=== ${name} ==="
        if "$@"; then
            log "--- ${name} OK ---"
        else
            warn "--- ${name} FAILED ---"
            failures+=("${name}")
        fi
    }

    # Primary build/test matrix: gcc-14 × {Debug, Release}.  We do NOT run
    # the clang build/test through `cmd_build clang`: clang-18 + system
    # libstdc++14 does not expose `std::expected` (needed by base/Error.h)
    # because `__cpp_lib_expected` is not yet enabled on that pairing.  The
    # clang exposure we *do* care about — sanitizer coverage — happens in
    # `cmd_sanitize` below, which configures clang with libc++ instead.
    _try "format-check"       cmd_format_check
    _try "build gcc Debug"    cmd_build gcc Debug
    _try "test gcc Debug"     cmd_test  gcc Debug
    _try "build gcc Release"  cmd_build gcc Release
    _try "test gcc Release"   cmd_test  gcc Release
    _try "sanitize"           cmd_sanitize
    _try "coverage"           cmd_coverage
    if command -v docker >/dev/null; then
        _try "docker-build"   cmd_docker_build
    else
        warn "docker-build: skipped (docker not installed)"
    fi

    printf '\n\033[1m==== ci.sh all summary ====\033[0m\n'
    if (( ${#failures[@]} == 0 )); then
        printf '\033[1;32mAll jobs passed.\033[0m\n'
        return 0
    fi
    printf '\033[1;31mFailures: %s\033[0m\n' "${failures[*]}"
    return 1
}

cmd_help () {
    sed -n '3,21p' "${BASH_SOURCE[0]}"
}

# --- Dispatcher ----------------------------------------------------------

main () {
    if (( $# == 0 )); then
        cmd_help
        exit 1
    fi
    local sub="$1"; shift
    case "${sub}" in
        build)        cmd_build        "$@" ;;
        test)         cmd_test         "$@" ;;
        sanitize)     cmd_sanitize     "$@" ;;
        coverage)     cmd_coverage     "$@" ;;
        format-check) cmd_format_check "$@" ;;
        format-fix)   cmd_format_fix   "$@" ;;
        docker-build) cmd_docker_build "$@" ;;
        cluster-test) cmd_cluster_test "$@" ;;
        all)          cmd_all          "$@" ;;
        help|-h|--help) cmd_help; exit 0 ;;
        *) die "unknown subcommand '${sub}' — run 'scripts/ci.sh help'" ;;
    esac
}

main "$@"
