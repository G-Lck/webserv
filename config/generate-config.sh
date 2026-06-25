#!/bin/bash

mkdir -p configs/valid configs/invalid

# ─── VALID 1: simple single server ───────────────────────────────────────────
cat > configs/valid/valid1.conf << 'EOF'
root /var/www/html;
index index.html;
autoindex off;
client_max_body_size 10000;

server {
    listen 8080;
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

echo "Generated:"
echo "  configs/valid/   -> 3 files"
echo "  configs/invalid/ -> 10 files"