#!/usr/bin/env python3

import os
import sys
import time


def normal_response():
    print("Content-Type: text/html")
    print()
    print("<html>")
    print("<body>")
    print("<h1>Hello from CGI</h1>")
    print("</body>")
    print("</html>")


def status_response():
    print("Status: 404 Not Found")
    print("Content-Type: text/html")
    print()
    print("<h1>404 - Page not found</h1>")


def redirect_response():
    print("Location: /login")
    print()


def redirect_with_status():
    print("Status: 301 Moved Permanently")
    print("Location: https://example.com")
    print()


def json_response():
    print("Content-Type: application/json")
    print()
    print('{"status":"ok"}')


def empty_body():
    print("Content-Type: text/plain")
    print()


def binary_response():
    sys.stdout.buffer.write(b"Content-Type: application/octet-stream\n\n")
    sys.stdout.buffer.write(bytes([0, 1, 2, 255, 100, 50]))


def malformed_header():
    print("Content-Type text/html")
    print()
    print("This should become 502")


def no_content_type():
    print("Hello")
    print()


def garbage_before_headers():
    print("THIS IS GARBAGE")
    print("Content-Type: text/html")
    print()
    print("<h1>Broken CGI</h1>")


def slow_response():
    print("Content-Type: text/html")
    print()
    print("<h1>Sleeping...</h1>")
    sys.stdout.flush()

    time.sleep(10)

    print("<h1>Finished</h1>")


def show_environment():
    print("Content-Type: text/html")
    print()
    print("<html><body>")

    for key, value in os.environ.items():
        print("<p>%s = %s</p>" % (key, value))

    print("</body></html>")

def get_response():
    print("Content-Type: text/html")
    print()
    print("<h1>GET request</h1>")
    print("Method: " + os.environ.get("REQUEST_METHOD", ""))


def post_response():
    print("Content-Type: text/html")
    print()

    length = int(os.environ.get("CONTENT_LENGTH", "0"))
    body = sys.stdin.read(length)

    print("<h1>POST request</h1>")
    print("Method: " + os.environ.get("REQUEST_METHOD", ""))
    print("Body: " + body)


def relative_file_response():
    print("Content-Type: text/plain")
    print()

    try:
        with open("test.txt", "r") as f:
            print("Relative file works!")
            print(f.read())
    except Exception as e:
        print("Relative file failed:")
        print(str(e))


def error_response():
    print("Content-Type: text/html")
    print()
    print("<h1>CGI error test</h1>")

    raise Exception("Intentional CGI error")


def infinite_loop():
    print("Content-Type: text/html")
    print()
    print("<h1>CGI infinite loop test</h1>")
    sys.stdout.flush()

    while True:
        time.sleep(1)

# Change this to test different cases

test = "redirect"


if test == "normal":
    normal_response()

elif test == "status":
    status_response()

elif test == "redirect":
    redirect_response()

elif test == "redirect_status":
    redirect_with_status()

elif test == "json":
    json_response()

elif test == "empty":
    empty_body()

elif test == "binary":
    binary_response()

elif test == "malformed":
    malformed_header()

elif test == "no_content_type":
    no_content_type()

elif test == "garbage":
    garbage_before_headers()

elif test == "slow":
    slow_response()

elif test == "env":
    show_environment()

elif test == "get":
    get_response()

elif test == "post":
    post_response()

elif test == "relative":
    relative_file_response()

elif test == "error":
    error_response()

elif test == "infinite":
    infinite_loop()