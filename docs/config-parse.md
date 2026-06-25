# Webserv Config Directives Reference

---

## GlobalConfig

### `_root`
- Type: `std::string`
- Optional, only **one** allowed — duplicate = error
- Must be an absolute path starting with `/`
- If missing and server has no root, request will fail
- Trailing slash optional: `/var/www/html` and `/var/www/html/` both valid
- Edge cases:
  - Path doesn't exist → serve errors at runtime, not parse time
  - Relative path → behavior undefined, always enforce absolute

---

### `_index`
- Type: `std::vector<std::string>`
- Optional, multiple files allowed
- Order matters: first file found is served
- Files are relative to `root`
- Edge cases:
  - Empty directive (`index ;`) → error
  - File with path separator (`index /abs/path.html`) → nginx allows it, be consistent
  - If none found → falls back to autoindex if on, else 403

---

### `_error_pages`
- Type: `std::map<int, std::string>`
- Key = HTTP error code, Value = path to page
- Multiple codes can map to same path: `error_page 500 502 503 /50x.html`
- Valid codes: **400–599** only
- Path is relative to `root`
- Edge cases:
  - Code outside 400–599 → reject at parse time
  - Non-numeric code → reject
  - Missing path → error
  - Path doesn't start with `/` → still valid (relative to root)
  - Same code defined twice → second overrides first, or throw error (your choice, be consistent)

---

### `_autoindex`
- Type: `bool`
- Optional, default: `off`
- Valid values: `on` / `off` only
- Edge cases:
  - Any other value → error
  - If on and index file found → index file takes priority, autoindex ignored

---

### `_client_max_body_size`
- Type: `int`
- Optional, default: `1MB` (1000000 bytes) in nginx
- Value in bytes, must be a positive integer
- `0` = no limit
- Edge cases:
  - Negative number → reject
  - Non-numeric → reject
  - Very large value → int overflow risk, consider `long`
  - Nginx supports suffixes like `1m`, `10k` — decide if you support this

---

### `_servers`
- Type: `std::vector<ServerConfig>`
- At least **one** server required
- Edge cases:
  - Empty config (no servers) → error
  - Two servers on exact same `host:port` with no `server_name` → conflict

---

## ServerConfig

### `_listen`
- Type: `std::vector<std::pair<std::string, std::string>>` (host, port)
- At least **one** required
- Formats accepted:
  - `8080` → host defaults to `0.0.0.0`
  - `127.0.0.1:8080`
  - `localhost:8080`
- Port range: **1–65535** (ports < 1024 require root)
- Multiple `listen` directives per server are valid
- Edge cases:
  - Port 0 → invalid
  - Port > 65535 → invalid
  - Non-numeric port → reject
  - Duplicate `host:port` within same server → reject
  - `0.0.0.0` and specific IP on same port → conflict at bind time
  - Host part validation: only digits and dots for IP, alphanumeric + `-` + `.` for hostname

---

### `_server_name`
- Type: `std::vector<std::string>`
- Optional, multiple names allowed per directive
- `_` = catch-all (special nginx convention)
- Empty = accepts all requests (default nginx behavior)
- Edge cases:
  - Duplicate names across servers on same port → first match wins
  - Special chars except `-` and `.` → reject
  - `_` underscore is valid (catch-all), don't reject it
  - Numbers only like `123` → technically valid hostname
  - Very long names → no hard limit but be reasonable

---

### `_root`
- Same rules as GlobalConfig `_root`
- Overrides global root for this server
- One per server block

---

### `_index`
- Same rules as GlobalConfig `_index`
- Overrides global index for this server
- Multiple files allowed

---

### `_error_pages`
- Same rules as GlobalConfig `_error_pages`
- Overrides global error pages for matching codes
- Non-matching codes fall back to global

---

### `_autoindex`
- Same rules as GlobalConfig `_autoindex`
- Overrides global for this server

---

### `_client_max_body_size`
- Same rules as GlobalConfig `_client_max_body_size`
- Overrides global for this server

---

### `_cgi_handler`
- Type: `std::string` (format: `ext path`, e.g. `.php /usr/bin/php`)
- Optional
- Only **one** per server (or consider making it a map)
- Edge cases:
  - Extension must start with `.`
  - Path must be absolute (start with `/`)
  - Path must point to a valid executable — not validated at parse time
  - Duplicate CGI directive → reject or override, be consistent
  - No space between ext and path → parse error
  - Consider making it `std::map<std::string, std::string>` for multiple CGI handlers

---

### `_locations`
- Type: `std::vector<LocationConfig>`
- Optional, multiple allowed
- Edge cases:
  - Duplicate paths → reject or last wins, be consistent
  - Nested locations → depends on your implementation
  - Path must start with `/`

---

## LocationConfig

### `_path`
- Type: `std::string`
- **Required**, must start with `/`
- Edge cases:
  - Path without leading `/` → reject
  - Trailing slash matters: `/path` and `/path/` are different in nginx
  - Empty path → reject

---

### `_root`
- Same rules as ServerConfig `_root`
- Overrides server root for this location
- One per location block

---

### `_index`
- Same rules as ServerConfig `_index`
- Overrides server index for this location

---

### `_error_pages`
- Type: `std::map<int, std::string>`
- Same rules as ServerConfig `_error_pages`
- Overrides server error pages for matching codes

---

### `_autoindex`
- Same rules as ServerConfig `_autoindex`
- Overrides server for this location

---

### `_client_max_body_size`
- Same rules as ServerConfig `_client_max_body_size`
- Overrides server for this location

---

### `_cgi_handler`
- Same rules as ServerConfig `_cgi_handler`
- Overrides server CGI for this location
- Edge cases:
  - CGI runs in its own directory — make sure path is correct
  - If file extension doesn't match → CGI not triggered
  - CGI timeout not required by subject but good to consider

---

### `_allow_methods`
- Type: `std::vector<std::string>`
- Optional, empty = all methods allowed
- Valid values per subject: `GET`, `POST`, `DELETE` only
- Edge cases:
  - `HEAD` is implicitly allowed when `GET` is allowed (nginx behavior)
  - Unknown method like `PUT` → reject at parse time
  - Case sensitive: `get` ≠ `GET` → enforce uppercase or normalize
  - Duplicate methods → deduplicate or reject
  - Empty directive (`allow_methods ;`) → error

---

### `_return_code`
- Type: `int`
- Valid redirect codes: **301**, **302**, **307**, **308**
- Must always come with `_return_url`
- Edge cases:
  - Code without URL → reject
  - Non-redirect code (e.g. 200) → reject or allow (your choice)
  - Non-numeric → reject

---

### `_return_url`
- Type: `std::string`
- Must always come with `_return_code`
- Can be absolute URL or relative path
- Edge cases:
  - URL without code → reject
  - Empty string → reject
  - No leading `/` for relative path → technically valid but ambiguous

---

### `_upload_path`
- Type: `std::string`
- Optional, only relevant when `POST` is in `_allow_methods`
- Directory where uploaded files are stored
- Edge cases:
  - Path doesn't exist → runtime error, not parse error
  - Must be absolute path
  - No write permission → runtime error
  - If POST allowed but no upload path → decide if you default to root or error

---

## General Edge Cases

- **Inheritance**: location inherits from server, server inherits from global. Missing directive = use parent's value
- **Empty blocks**: `server {}` with no directives → valid syntax but likely runtime error
- **Semicolons**: every directive must end with `;`, missing = parse error
- **Unknown directives**: reject with clear error message showing the token
- **File not found**: detected at runtime, not parse time (except CGI path optionally)
- **Unclosed brackets**: detected by `validBrackets()` before tokenizing
- **Comment lines**: lines starting with `#` are ignored
- **Blank lines**: ignored