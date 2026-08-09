#!/usr/bin/env python3

import json
import os
import sys
import uuid
from datetime import datetime, timezone
from pathlib import Path

UPLOADS_DIR = Path(__file__).resolve().parent.parent / "uploads"
ALLOWED_EXTENSIONS = {".avif", ".gif", ".jpeg", ".jpg", ".png", ".webp"}


def send_json(payload, status=None):
    if status:
        print("Status: " + status)
    print("Content-Type: application/json")
    print("Cache-Control: no-store")
    print()
    print(json.dumps(payload))


def error(message, status):
    send_json({"error": message}, status)


def disposition_parameter(disposition, parameter_name):
    for parameter in disposition.split(";")[1:]:
        key, separator, value = parameter.strip().partition("=")
        if separator and key.lower() == parameter_name:
            return value.strip().strip('"')
    return ""


def parse_multipart():
    content_type = os.environ.get("CONTENT_TYPE", "")
    if not content_type.lower().startswith("multipart/form-data"):
        raise ValueError("Expected multipart/form-data.")
    boundary = disposition_parameter(content_type, "boundary")
    if not boundary:
        raise ValueError("Multipart boundary is missing.")

    length = int(os.environ.get("CONTENT_LENGTH", "0"))
    body = sys.stdin.buffer.read(length)
    delimiter = b"--" + boundary.encode("ascii")
    if not body.startswith(delimiter):
        raise ValueError("Malformed multipart body.")

    fields = {}
    for part in body.split(delimiter)[1:]:
        if part.startswith(b"--"):
            break
        if not part.startswith(b"\r\n"):
            raise ValueError("Multipart part does not start with CRLF.")
        part = part[2:]
        headers, separator, data = part.partition(b"\r\n\r\n")
        if not separator:
            raise ValueError("Multipart part has no header separator.")
        if data.endswith(b"\r\n"):
            data = data[:-2]

        disposition = ""
        for header in headers.decode("latin-1").split("\r\n"):
            key, separator, value = header.partition(":")
            if separator and key.lower() == "content-disposition":
                disposition = value.strip()
        name = disposition_parameter(disposition, "name")
        if not name:
            raise ValueError("Multipart field name is missing.")
        filename = disposition_parameter(disposition, "filename")
        fields[name] = (filename, data)
    return fields


def get_text_field(fields, name):
    field = fields.get(name)
    if not field or field[0]:
        return ""
    return field[1].decode("utf-8", "replace").strip()


def safe_extension(filename):
    extension = Path(os.path.basename(filename)).suffix.lower()
    if extension not in ALLOWED_EXTENSIONS:
        return ""
    return extension


def upload_bird():
    try:
        fields = parse_multipart()
    except (ValueError, UnicodeError):
        error("Malformed multipart form.", "400 Bad Request")
        return

    image = fields.get("image")
    if not image or not image[0]:
        error("An image is required.", "400 Bad Request")
        return

    extension = safe_extension(image[0])
    if not extension:
        error("Unsupported image format.", "400 Bad Request")
        return

    name = get_text_field(fields, "name")
    comment = get_text_field(fields, "comment")
    if not name or not comment:
        error("A bird name and comment are required.", "400 Bad Request")
        return

    UPLOADS_DIR.mkdir(parents=True, exist_ok=True)
    filename = uuid.uuid4().hex + extension
    image_path = UPLOADS_DIR / filename
    with image_path.open("wb") as destination:
        destination.write(image[1])

    metadata = {
        "filename": filename,
        "name": name,
        "comment": comment,
        "uploaded_at": datetime.now(timezone.utc).isoformat()
    }
    with (UPLOADS_DIR / (filename + ".json")).open("w", encoding="utf-8") as metadata_file:
        json.dump(metadata, metadata_file)

    send_json(metadata, "201 Created")


def list_birds():
    birds = []
    if UPLOADS_DIR.is_dir():
        for metadata_path in UPLOADS_DIR.glob("*.json"):
            try:
                with metadata_path.open(encoding="utf-8") as metadata_file:
                    bird = json.load(metadata_file)
                filename = bird.get("filename", "")
                if safe_extension(filename) and (UPLOADS_DIR / filename).is_file():
                    birds.append({
                        "filename": filename,
                        "name": bird.get("name", ""),
                        "comment": bird.get("comment", ""),
                        "uploaded_at": bird.get("uploaded_at", "")
                    })
            except (OSError, ValueError, TypeError):
                continue
    birds.sort(key=lambda bird: bird["uploaded_at"], reverse=True)
    send_json({"birds": birds})


def main():
    method = os.environ.get("REQUEST_METHOD", "GET")
    if method == "POST":
        upload_bird()
    elif method == "GET":
        list_birds()
    else:
        error("Method not allowed.", "405 Method Not Allowed")


if __name__ == "__main__":
    main()
