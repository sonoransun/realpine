# shellcheck shell=bash
# Alpine CI environment helpers — toolchain probing so ci.sh runs the
# highest-available GCC / Clang / clang-format / lcov version on whatever
# host it happens to land on (local dev box, Ubuntu runner, ephemeral VM).

ci_first_present () {
    for candidate in "$@"; do
        if command -v "${candidate}" >/dev/null 2>&1; then
            command -v "${candidate}"
            return 0
        fi
    done
    return 1
}

ci_gcc_c () {
    ci_first_present gcc-14 gcc-13 gcc || echo /usr/bin/false
}

ci_gcc_cxx () {
    ci_first_present g++-14 g++-13 g++ || echo /usr/bin/false
}

ci_clang_c () {
    ci_first_present clang-18 clang-17 clang-16 clang || echo /usr/bin/false
}

ci_clang_cxx () {
    ci_first_present clang++-18 clang++-17 clang++-16 clang++ || echo /usr/bin/false
}

ci_clang_format () {
    ci_first_present clang-format-18 clang-format-17 clang-format-16 clang-format || echo /usr/bin/false
}

ci_clang_tidy () {
    ci_first_present clang-tidy-18 clang-tidy-17 clang-tidy-16 clang-tidy || echo /usr/bin/false
}

ci_report_env () {
    printf '  gcc:           %s\n' "$(ci_gcc_c)"
    printf '  g++:           %s\n' "$(ci_gcc_cxx)"
    printf '  clang:         %s\n' "$(ci_clang_c)"
    printf '  clang++:       %s\n' "$(ci_clang_cxx)"
    printf '  clang-format:  %s\n' "$(ci_clang_format)"
    printf '  clang-tidy:    %s\n' "$(ci_clang_tidy)"
    printf '  cmake:         %s\n' "$(command -v cmake || echo MISSING)"
    printf '  lcov:          %s\n' "$(command -v lcov  || echo MISSING)"
    printf '  docker:        %s\n' "$(command -v docker || echo MISSING)"
}
