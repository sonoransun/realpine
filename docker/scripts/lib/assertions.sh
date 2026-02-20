#!/bin/bash
# assertions.sh — Test assertion helpers

PASS_COUNT=0
FAIL_COUNT=0
CURRENT_TEST=""

# Begin a named test scenario.
begin_test() {
    CURRENT_TEST="$1"
    echo ""
    echo "=== TEST: $CURRENT_TEST ==="
}

# Record a pass.
pass() {
    local msg="${1:-$CURRENT_TEST}"
    echo "  PASS: $msg"
    ((PASS_COUNT++))
}

# Record a failure.
fail() {
    local msg="${1:-$CURRENT_TEST}"
    echo "  FAIL: $msg"
    ((FAIL_COUNT++))
}

# Assert two values are equal.
# Usage: assert_eq <expected> <actual> [message]
assert_eq() {
    local expected="$1"
    local actual="$2"
    local msg="${3:-expected '$expected', got '$actual'}"
    if [[ "$actual" == "$expected" ]]; then
        pass "$msg"
    else
        fail "$msg (expected='$expected' actual='$actual')"
    fi
}

# Assert actual string contains needle.
# Usage: assert_contains <haystack> <needle> [message]
assert_contains() {
    local haystack="$1"
    local needle="$2"
    local msg="${3:-string contains '$needle'}"
    if [[ "$haystack" == *"$needle"* ]]; then
        pass "$msg"
    else
        fail "$msg (not found in: '${haystack:0:200}')"
    fi
}

# Assert a JSON field has the expected value.
# Usage: assert_json_field <json> <field> <expected> [message]
assert_json_field() {
    local json="$1"
    local field="$2"
    local expected="$3"
    local msg="${4:-JSON .$field == '$expected'}"
    local actual
    actual=$(echo "$json" | jq -r ".$field" 2>/dev/null)
    assert_eq "$expected" "$actual" "$msg"
}

# Assert actual >= min (numeric).
# Usage: assert_ge <actual> <min> [message]
assert_ge() {
    local actual="$1"
    local min="$2"
    local msg="${3:-$actual >= $min}"
    if [[ "$actual" -ge "$min" ]] 2>/dev/null; then
        pass "$msg"
    else
        fail "$msg (actual=$actual, min=$min)"
    fi
}

# Assert an HTTP GET returns 200.
# Usage: assert_http_ok <url> [message]
assert_http_ok() {
    local url="$1"
    local msg="${2:-HTTP 200 from $url}"
    local code
    code=$(curl -sf -o /dev/null -w '%{http_code}' --connect-timeout 3 "$url" 2>/dev/null)
    assert_eq "200" "$code" "$msg"
}

# Retry a command up to max_attempts times with a sleep between.
# Usage: retry_assert <max_attempts> <sleep_sec> <command...>
retry_assert() {
    local max="$1"
    local sleep_sec="$2"
    shift 2

    for attempt in $(seq 1 "$max"); do
        if "$@" 2>/dev/null; then
            return 0
        fi
        if [[ "$attempt" -lt "$max" ]]; then
            sleep "$sleep_sec"
        fi
    done
    return 1
}

# Print test summary and return exit code.
test_summary() {
    echo ""
    echo "========================================"
    echo "  Results: $PASS_COUNT passed, $FAIL_COUNT failed"
    echo "========================================"
    [[ "$FAIL_COUNT" -eq 0 ]]
}
