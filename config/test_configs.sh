#!/bin/bash
# Compiles WebServ and runs it against every config in configs/valid and
# configs/invalid using the --test-config flag (parses + prints config,
# never starts the server). Pass/fail is read from the parser's own
# "Parser passed."/"Parser Failed." output (see srcs/utils/utils.cpp).

set -u

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

BIN="./WebServ"
VALID_DIR="config/configs/valid"
INVALID_DIR="config/configs/invalid"

GREEN='\033[32m'
RED='\033[31m'
YELLOW='\033[33m'
RESET='\033[0m'

echo -e "${YELLOW}== Building ${BIN} ==${RESET}"
if ! make -j"$(nproc)"; then
	echo -e "${RED}Build failed, aborting.${RESET}"
	exit 1
fi

pass_count=0
fail_count=0

# $1 = config file, $2 = expected result ("pass" or "fail")
run_config()
{
	local conf="$1"
	local expected="$2"
	local output
	local result

	output="$("$BIN" "$conf" --test-config 2>&1)"

	if echo "$output" | grep -q "Parser passed."; then
		result="pass"
	elif echo "$output" | grep -q "Parser Failed."; then
		result="fail"
	else
		result="unknown"
	fi

	if [ "$result" = "$expected" ]; then
		echo -e "${GREEN}[OK]${RESET}   $conf (expected: $expected, got: $result)"
		pass_count=$((pass_count + 1))
	else
		echo -e "${RED}[FAIL]${RESET} $conf (expected: $expected, got: $result)"
		fail_count=$((fail_count + 1))
		echo "$output" | sed 's/^/       | /'
	fi
}

echo
echo -e "${YELLOW}== Valid configs (expected: pass) ==${RESET}"
for conf in "$VALID_DIR"/*.conf; do
	[ -e "$conf" ] || continue
	run_config "$conf" "pass"
done

echo
echo -e "${YELLOW}== Invalid configs (expected: fail) ==${RESET}"
for conf in "$INVALID_DIR"/*.conf; do
	[ -e "$conf" ] || continue
	run_config "$conf" "fail"
done

echo
echo -e "${YELLOW}== Summary ==${RESET}"
echo -e "${GREEN}Passed: $pass_count${RESET}"
echo -e "${RED}Failed: $fail_count${RESET}"

[ "$fail_count" -eq 0 ]
