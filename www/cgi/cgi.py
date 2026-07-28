#!/usr/bin/env python3
import sys

payload = sys.stdin.read()

response = (
    "Content-Type: text/plain\r\n"
    f"Content-Length: {len(payload)}\r\n"
    "\r\n"
    f"{payload}"
)

sys.stdout.write(response)
sys.stdout.flush()