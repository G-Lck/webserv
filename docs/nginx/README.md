# Nginx test bench (docs/nginx)

This folder lets you quickly test an Nginx config, send HTTP requests, and read error/access logs.

## Files

- `configs/basic.conf`: valid example config (port `8088`)
- `configs/invalid_syntax.conf`: intentionally invalid config to test parsing errors
- `bench.sh`: main script
- `logs/`: Nginx logs (created automatically)

## Prerequisites

- `nginx` installed and available in `PATH`
- `curl` for HTTP requests
- `nc` (netcat) for malformed raw requests

## Quick usage

From the project root:

```bash
cd docs/nginx
chmod +x bench.sh

# check a config
./bench.sh check configs/basic.conf

# start nginx with this config
./bench.sh start configs/basic.conf

# send requests
./bench.sh request GET /
./bench.sh request GET /missing
./bench.sh request GET /boom
./bench.sh request POST /echo "hello"

# raw request (useful to test parsing errors)
./bench.sh raw "BAD / HTTP/1.1\r\nHost: localhost\r\n\r\n"

# read logs
./bench.sh logs error 80
./bench.sh logs access 40

# stop nginx
./bench.sh stop configs/basic.conf
```

## Automated tests

```bash
cd docs/nginx
./bench.sh test configs/basic.conf
```

The `test` mode does:
1. stop + start nginx
2. GET `/`
3. GET `/missing`
4. GET `/boom`
5. send a malformed raw request
6. display error/access logs
7. stop nginx

## Test config errors

```bash
cd docs/nginx
./bench.sh check configs/invalid_syntax.conf
```

You will see the Nginx syntax error directly in the command output.

## Add your own config

1. Add a file in `configs/` (example: `configs/my.conf`)
2. Run:

```bash
./bench.sh check configs/my.conf
./bench.sh start configs/my.conf
```

If your `listen` is not `127.0.0.1:8088`, pass host/port to the request command:

```bash
./bench.sh request GET / "" 127.0.0.1 9090
```
