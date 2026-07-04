#!/usr/bin/env bash
set -u

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
DEFAULT_CFG="configs/basic.conf"

usage() {
    cat <<'EOF'
Usage:
  ./bench.sh check [config]
  ./bench.sh start [config]
  ./bench.sh stop [config]
  ./bench.sh restart [config]
  ./bench.sh request METHOD PATH [BODY] [HOST] [PORT]
  ./bench.sh raw "RAW_HTTP_REQUEST" [HOST] [PORT]
  ./bench.sh logs [error|access] [lines]
  ./bench.sh test [config]

Examples:
  ./bench.sh check configs/basic.conf
  ./bench.sh start configs/basic.conf
  ./bench.sh request GET /
  ./bench.sh request POST /echo "hello"
  ./bench.sh logs error 50
  ./bench.sh stop configs/basic.conf
EOF
}

require_nginx() {
    if ! command -v nginx >/dev/null 2>&1; then
        echo "Error: nginx is not installed or not in PATH." >&2
        exit 1
    fi
}

cfg_path() {
    local cfg="${1:-$DEFAULT_CFG}"
    if [[ "$cfg" != /* ]]; then
        cfg="$ROOT_DIR/$cfg"
    fi
    echo "$cfg"
}

check_cfg() {
    local cfg
    cfg="$(cfg_path "$1")"
    require_nginx
    mkdir -p "$ROOT_DIR/logs"
    nginx -t -p "$ROOT_DIR" -c "$cfg"
}

start_nginx() {
    local cfg
    cfg="$(cfg_path "$1")"
    require_nginx
    mkdir -p "$ROOT_DIR/logs"

    if ! check_cfg "$cfg"; then
        echo "Config check failed, nginx not started." >&2
        exit 1
    fi

    nginx -p "$ROOT_DIR" -c "$cfg"
    echo "Nginx started with config: $cfg"
}

stop_nginx() {
    local cfg
    cfg="$(cfg_path "$1")"
    require_nginx
    nginx -s stop -p "$ROOT_DIR" -c "$cfg" >/dev/null 2>&1 || true
    echo "Stop signal sent (if nginx was running with this config)."
}

request_http() {
    local method="${1:-GET}"
    local path="${2:-/}"
    local body="${3:-}"
    local host="${4:-127.0.0.1}"
    local port="${5:-8088}"

    local headers_file body_file
    headers_file="$(mktemp)"
    body_file="$(mktemp)"

    if [[ -n "$body" ]]; then
        curl -sS -D "$headers_file" -o "$body_file" -X "$method" "http://$host:$port$path" --data "$body"
    else
        curl -sS -D "$headers_file" -o "$body_file" -X "$method" "http://$host:$port$path"
    fi

    echo "==== Response headers ===="
    cat "$headers_file"
    echo "==== Response body ===="
    cat "$body_file"

    rm -f "$headers_file" "$body_file"
}

raw_http() {
    local payload="${1:-}"
    local host="${2:-127.0.0.1}"
    local port="${3:-8088}"

    if [[ -z "$payload" ]]; then
        echo "Error: raw payload is empty." >&2
        exit 1
    fi

    if command -v nc >/dev/null 2>&1; then
        printf "%b" "$payload" | nc "$host" "$port"
    else
        echo "Error: nc (netcat) is required for raw requests." >&2
        exit 1
    fi
}

show_logs() {
    local kind="${1:-error}"
    local lines="${2:-40}"
    local file

    if [[ "$kind" == "access" ]]; then
        file="$ROOT_DIR/logs/access.log"
    else
        file="$ROOT_DIR/logs/error.log"
    fi

    if [[ ! -f "$file" ]]; then
        echo "No log file yet: $file"
        exit 0
    fi

    tail -n "$lines" "$file"
}

run_test_suite() {
    local cfg="${1:-$DEFAULT_CFG}"
    stop_nginx "$cfg"
    start_nginx "$cfg"

    echo
    echo "[1/4] GET /"
    request_http GET /

    echo
    echo "[2/4] GET /missing"
    request_http GET /missing

    echo
    echo "[3/4] GET /boom"
    request_http GET /boom

    echo
    echo "[4/4] Raw malformed request"
    raw_http "BAD / HTTP/1.1\r\nHost: localhost\r\n\r\n" || true

    echo
    echo "Last error log lines:"
    show_logs error 60

    echo
    echo "Last access log lines:"
    show_logs access 20

    stop_nginx "$cfg"
}

cmd="${1:-}"
case "$cmd" in
    check)
        check_cfg "${2:-$DEFAULT_CFG}"
        ;;
    start)
        start_nginx "${2:-$DEFAULT_CFG}"
        ;;
    stop)
        stop_nginx "${2:-$DEFAULT_CFG}"
        ;;
    restart)
        stop_nginx "${2:-$DEFAULT_CFG}"
        start_nginx "${2:-$DEFAULT_CFG}"
        ;;
    request)
        request_http "${2:-GET}" "${3:-/}" "${4:-}" "${5:-127.0.0.1}" "${6:-8088}"
        ;;
    raw)
        raw_http "${2:-}" "${3:-127.0.0.1}" "${4:-8088}"
        ;;
    logs)
        show_logs "${2:-error}" "${3:-40}"
        ;;
    test)
        run_test_suite "${2:-$DEFAULT_CFG}"
        ;;
    *)
        usage
        exit 1
        ;;
esac
