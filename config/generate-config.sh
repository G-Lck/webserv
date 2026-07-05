#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")" || exit 1

mkdir -p configs/valid configs/invalid

# ─── VALID 1: simple single server ───────────────────────────────────────────
cat > configs/valid/valid1.conf << 'EOF'
root /var/www/html;
index index.html;
autoindex off;
client_max_body_size 10000;

server {
    listen 8085;
    server_name localhost;
    root /var/www/html;
    index index.html index.htm;
    error_page 404 /404.html;
    error_page 500 /50x.html;
    autoindex off;
    client_max_body_size 5000;
    cgi_handler .php /usr/bin/php;

    location / {
        limit_except GET POST;
        autoindex off;
        index index.html;
    }

    location /upload {
        limit_except POST;
        autoindex off;
    }
}
EOF

# ─── VALID 2: multiple servers ────────────────────────────────────────────────
cat > configs/valid/valid2.conf << 'EOF'
server {
    listen 8080;
    server_name example.com www.example.com;
    root /var/www/example;
    index index.html;
    error_page 404 /404.html;

    location / {
        limit_except GET;
        autoindex off;
    }

    location /api {
        limit_except GET POST DELETE;
        autoindex off;
    }
}

server {
    listen 8081;
    server_name test.com;
    root /var/www/test;
    index index.html;

    location / {
        limit_except GET POST;
        autoindex on;
    }
}
EOF

# ─── VALID 3: full featured ───────────────────────────────────────────────────
cat > configs/valid/valid3.conf << 'EOF'
root /var/www;
index index.html;
autoindex off;
client_max_body_size 100000;
error_page 404 /errors/404.html;
error_page 500 /errors/500.html;

server {
    listen 8080;
    listen 127.0.0.1:8081;
    server_name localhost _;
    root /var/www/main;
    index index.html index.htm;
    autoindex on;
    client_max_body_size 50000;
    cgi_handler .py /usr/bin/python3;

    location / {
        limit_except GET POST;
        index index.html;
        autoindex on;
    }

    location /static {
        limit_except GET;
        root /var/www/static;
        autoindex off;
    }

    location /upload {
        limit_except POST;
        client_max_body_size 100000;
    }

    location /redirect {
        return 301 /;
    }

    location /cgi-bin {
        limit_except GET POST;
        cgi_handler .py /usr/bin/python3;
        root /var/www/cgi;
    }
}
EOF

# ─── INVALID 1: missing semicolon ─────────────────────────────────────────────
cat > configs/invalid/invalid1_missing_semicolon.conf << 'EOF'
server {
    listen 8080
    server_name localhost;
    root /var/www/html;
}
EOF

# ─── INVALID 2: unclosed bracket ──────────────────────────────────────────────
cat > configs/invalid/invalid2_unclosed_bracket.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /var/www/html;
    location / {
        limit_except GET;
}
EOF

# ─── INVALID 3: wrong bracket order ───────────────────────────────────────────
cat > configs/invalid/invalid3_wrong_bracket_order.conf << 'EOF'
server }
    listen 8080;
    server_name localhost;
    root /var/www/html;
{
EOF

# ─── INVALID 4: invalid port ──────────────────────────────────────────────────
cat > configs/invalid/invalid4_invalid_port.conf << 'EOF'
server {
    listen 99999;
    server_name localhost;
    root /var/www/html;

    location / {
        limit_except GET;
    }
}
EOF

# ─── INVALID 5: invalid HTTP method in limit_except ──────────────────────────
cat > configs/invalid/invalid5_bad_method.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /var/www/html;

    location / {
        limit_except GET PUT PATCH;
    }
}
EOF

# ─── INVALID 6: duplicate root ────────────────────────────────────────────────
cat > configs/invalid/invalid6_duplicate_root.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /var/www/html;
    root /var/www/other;

    location / {
        limit_except GET;
    }
}
EOF

# ─── INVALID 7: invalid error page code ───────────────────────────────────────
cat > configs/invalid/invalid7_bad_error_code.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /var/www/html;
    error_page 999 /error.html;

    location / {
        limit_except GET;
    }
}
EOF

# ─── INVALID 8: CGI without absolute path ─────────────────────────────────────
cat > configs/invalid/invalid8_cgi_relative_path.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /var/www/html;
    cgi_handler .php usr/bin/php;

    location / {
        limit_except GET;
    }
}
EOF

# ─── INVALID 9: CGI extension without dot ─────────────────────────────────────
cat > configs/invalid/invalid9_cgi_no_dot.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /var/www/html;
    cgi_handler php /usr/bin/php;

    location / {
        limit_except GET;
    }
}
EOF

# ─── INVALID 10: location path without leading slash ─────────────────────────
cat > configs/invalid/invalid10_location_no_slash.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /var/www/html;

    location uploads {
        limit_except POST;
    }
}
EOF

# ─── VALID 4: grouped error codes sharing one path ────────────────────────────
cat > configs/valid/valid4_grouped_error_pages.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /var/www/html;
    error_page 500 502 503 504 /50x.html;

    location / {
        limit_except GET;
    }
}
EOF

# ─── VALID 5: server block with no global directives at all ──────────────────
cat > configs/valid/valid5_minimal_no_global_directives.conf << 'EOF'
server {
    listen 3000;
    root /var/www/minimal;

    location / {
        limit_except GET;
    }
}
EOF

# ─── VALID 6: nginx-style catch-all server_name ───────────────────────────────
cat > configs/valid/valid6_wildcard_server_name.conf << 'EOF'
server {
    listen 8080;
    server_name _;
    root /var/www/html;

    location / {
        limit_except GET;
    }
}
EOF

# ─── VALID 7: allow_methods alias + upload_path ───────────────────────────────
cat > configs/valid/valid7_allow_methods_and_upload_path.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /var/www/html;

    location /upload {
        allow_methods POST;
        upload_path /var/www/uploads;
    }
}
EOF

# ─── VALID 8: comments and blank lines interleaved ────────────────────────────
cat > configs/valid/valid8_comments_and_blank_lines.conf << 'EOF'
# Top level comment
root /var/www/html;
# another comment
index index.html;

server {
    # server-level comment
    listen 8080;
    server_name localhost;
    root /var/www/html;

    location / {
        limit_except GET;
    }
}
EOF

# ─── VALID 9: redirect-only location ──────────────────────────────────────────
cat > configs/valid/valid9_return_redirect_location.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /var/www/html;

    location /old-page {
        return 308 /new-page;
    }
}
EOF

# ─── INVALID 11: empty root value ─────────────────────────────────────────────
cat > configs/invalid/invalid11_empty_root_value.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root ;
}
EOF

# ─── INVALID 12: root value not absolute ──────────────────────────────────────
cat > configs/invalid/invalid12_root_not_absolute.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root var/www/html;
}
EOF

# ─── INVALID 13: error_page code below the accepted range ────────────────────
cat > configs/invalid/invalid13_error_code_out_of_range_low.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /var/www/html;
    error_page 200 /ok.html;

    location / {
        limit_except GET;
    }
}
EOF

# ─── INVALID 14: listen host with invalid characters ──────────────────────────
cat > configs/invalid/invalid14_listen_invalid_host_chars.conf << 'EOF'
server {
    listen bad_host:8080;
    server_name localhost;
    root /var/www/html;
}
EOF

# ─── INVALID 15: location missing opening brace ───────────────────────────────
cat > configs/invalid/invalid15_location_missing_open_brace.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /var/www/html;

    location /foo
    limit_except GET;
}
EOF

# ─── INVALID 16: return directive with disallowed code ────────────────────────
cat > configs/invalid/invalid16_return_invalid_code.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /var/www/html;

    location /old {
        return 200 /new;
    }
}
EOF

# ─── INVALID 17: upload_path not absolute ─────────────────────────────────────
cat > configs/invalid/invalid17_upload_path_not_absolute.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /var/www/html;

    location /upload {
        limit_except POST;
        upload_path uploads;
    }
}
EOF

# ─── INVALID 18: duplicate cgi_handler directive ──────────────────────────────
cat > configs/invalid/invalid18_cgi_duplicate.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /var/www/html;
    cgi_handler .php /usr/bin/php;
    cgi_handler .py /usr/bin/python3;

    location / {
        limit_except GET;
    }
}
EOF

echo "Generated:"
echo "  configs/valid/   -> 9 files"
echo "  configs/invalid/ -> 18 files"