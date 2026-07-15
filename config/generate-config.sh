#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")" || exit 1

mkdir -p configs/valid configs/invalid

# Paths like /www/... are resolved by the server as "." + path,
# i.e. relative to the project root at runtime.

PY_BIN=$(command -v python3 || echo /usr/bin/python3)

cat > configs/valid/valid1.conf << EOF
root /www;
index index.html;
autoindex off;
client_max_body_size 10000;

server {
    listen 8085;
    server_name localhost;
    error_page 404 /error/404.html;
    autoindex off;
    client_max_body_size 5000;
    cgi_handler .py $PY_BIN;

    location / {
        limit_except GET POST;
        autoindex off;
    }

    location /upload {
        limit_except POST;
        upload_path /www/uploads;
        autoindex off;
    }
}
EOF

cat > configs/valid/valid2.conf << 'EOF'
server {
    listen 8080;
    server_name example.com www.example.com;
    root /www;
    index index.html;
    error_page 404 /error/404.html;

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
    root /www;
    index index.html;

    location / {
        limit_except GET POST;
        autoindex on;
    }
}
EOF

cat > configs/valid/valid3.conf << EOF
root /www;
index index.html;
autoindex off;
client_max_body_size 100000;
error_page 404 /error/404.html;

server {
    listen 8080;
    listen 127.0.0.1:8081;
    server_name localhost _;
    root /www;
    index index.html;
    autoindex on;
    client_max_body_size 50000;
    cgi_handler .py $PY_BIN;

    location / {
        limit_except GET POST;
        autoindex on;
    }

    location /static {
        limit_except GET;
        root /www;
        autoindex off;
    }

    location /upload {
        limit_except POST;
        upload_path /www/uploads;
        client_max_body_size 100000;
    }

    location /redirect {
        return 301 /;
    }

    location /cgi-bin {
        limit_except GET POST;
        cgi_handler .py $PY_BIN;
        root /www;
    }
}
EOF

cat > configs/invalid/invalid1_missing_semicolon.conf << 'EOF'
server {
    listen 8080
    server_name localhost;
    root /www;
}
EOF

cat > configs/invalid/invalid2_unclosed_bracket.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /www;
    location / {
        limit_except GET;
}
EOF

cat > configs/invalid/invalid3_wrong_bracket_order.conf << 'EOF'
server }
    listen 8080;
    server_name localhost;
    root /www;
{
EOF

cat > configs/invalid/invalid4_invalid_port.conf << 'EOF'
server {
    listen 99999;
    server_name localhost;
    root /www;

    location / {
        limit_except GET;
    }
}
EOF

cat > configs/invalid/invalid5_bad_method.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /www;

    location / {
        limit_except GET PUT PATCH;
    }
}
EOF

cat > configs/invalid/invalid6_duplicate_root.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /www;
    root /www/other;

    location / {
        limit_except GET;
    }
}
EOF

cat > configs/invalid/invalid7_bad_error_code.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /www;
    error_page 999 /error.html;

    location / {
        limit_except GET;
    }
}
EOF

cat > configs/invalid/invalid8_cgi_relative_path.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /www;
    cgi_handler .php usr/bin/php;

    location / {
        limit_except GET;
    }
}
EOF

cat > configs/invalid/invalid9_cgi_no_dot.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /www;
    cgi_handler php /usr/bin/php;

    location / {
        limit_except GET;
    }
}
EOF

cat > configs/invalid/invalid10_location_no_slash.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /www;

    location uploads {
        limit_except POST;
    }
}
EOF

cat > configs/valid/valid5_minimal_no_global_directives.conf << 'EOF'
server {
    listen 3000;
    root /www;

    location / {
        limit_except GET;
    }
}
EOF

cat > configs/valid/valid6_wildcard_server_name.conf << 'EOF'
server {
    listen 8080;
    server_name _;
    root /www;

    location / {
        limit_except GET;
    }
}
EOF

cat > configs/valid/valid7_allow_methods_and_upload_path.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /www;

    location /upload {
        allow_methods POST;
        upload_path /www/uploads;
    }
}
EOF

cat > configs/valid/valid8_comments_and_blank_lines.conf << 'EOF'
# Top level comment
root /www;
# another comment
index index.html;

server {
    # server-level comment
    listen 8080;
    server_name localhost;

    location / {
        limit_except GET;
    }
}
EOF

cat > configs/valid/valid9_return_redirect_location.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /www;

    location /old-page {
        return 308 /new-page;
    }
}
EOF

cat > configs/invalid/invalid11_empty_root_value.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root ;
}
EOF

cat > configs/invalid/invalid12_root_not_absolute.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root www;
}
EOF

cat > configs/invalid/invalid13_error_code_out_of_range_low.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /www;
    error_page 200 /ok.html;

    location / {
        limit_except GET;
    }
}
EOF

cat > configs/invalid/invalid14_listen_invalid_host_chars.conf << 'EOF'
server {
    listen bad_host:8080;
    server_name localhost;
    root /www;
}
EOF

cat > configs/invalid/invalid15_location_missing_open_brace.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /www;

    location /foo
    limit_except GET;
}
EOF

cat > configs/invalid/invalid16_return_invalid_code.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /www;

    location /old {
        return 200 /new;
    }
}
EOF

cat > configs/invalid/invalid17_upload_path_not_absolute.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /www;

    location /upload {
        limit_except POST;
        upload_path uploads;
    }
}
EOF

cat > configs/invalid/invalid18_cgi_duplicate.conf << EOF
server {
    listen 8080;
    server_name localhost;
    root /www;
    cgi_handler .py $PY_BIN;
    cgi_handler .py $PY_BIN;

    location / {
        limit_except GET;
    }
}
EOF

cat > configs/invalid/invalid19_root_does_not_exist.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /www/does_not_exist;

    location / {
        limit_except GET;
    }
}
EOF

cat > configs/invalid/invalid20_upload_path_does_not_exist.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /www;

    location /upload {
        limit_except POST;
        upload_path /www/no_such_dir;
    }
}
EOF

cat > configs/invalid/invalid21_cgi_interpreter_missing.conf << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root /www;
    cgi_handler .php /usr/bin/definitely_not_a_real_binary;

    location / {
        limit_except GET;
    }
}
EOF

echo "Generated:"
echo "  configs/valid/   -> 8 files"
echo "  configs/invalid/ -> 21 files"