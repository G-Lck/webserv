*This project has been created as part of the 42 curriculum by glucken, rspinell.*

# WebServ

## Description

WebServ is our own simplified implementation of an HTTP/1.1 server, following the general rules and behavior of NGINX.

The project consists of creating a server that takes a `.config` file as an argument. After parsing the configuration file, WebServ uses the information provided to set up its servers, listening sockets, ports, routes, and other settings.

Clients can then connect to the configured `host:port` endpoints using HTTP, send requests, and receive responses.

WebServ also supports **CGI (Common Gateway Interface)**. CGI allows the server to execute an external program to handle a request and generate dynamic content. Instead of only serving static files, WebServ can pass a request to a CGI program, which processes it and returns a response to the client.

## Instructions

All commands should be executed from the root of the project.

### Compilation

Run:

```bash
make
```

This will compile the project and create the `WebServ` executable.

### Running the server

Launch WebServ by providing a configuration file as the first argument:

```bash
./WebServ <config_file>
```

For example:

```bash
./WebServ config/real/exam-config
```

The server will then listen on the addresses and ports specified in the configuration file.
You can connect to the different servers using a browser or tools such as `curl`, depending on the `host:port` configured.
For example, if the configuration contains:

```
listen 127.0.0.1:8080;
```

you can access it through:

```
http://127.0.0.1:8080
```

### Disabling logs

Logs can be disabled using the `--no-log` flag. This is useful when running stress tests with tools such as `siege`, where generating a large number of log entries is unnecessary.

```bash
./WebServ <config_file> --no-log
```

### Test Config Parser

You can run a generated tester `test_configs.sh`. This will test all the files created by `test_configs.sh`.

```bash
./config/test_configs.sh
```

## Resources

### HTTP

- [RFC 9110 — HTTP Semantics](https://www.rfc-editor.org/rfc/rfc9110.html)
  Defines the general semantics of HTTP, including methods, status codes, headers, and other key concepts.
- [RFC 9112 — HTTP/1.1](https://www.rfc-editor.org/rfc/rfc9112.html)
  Defines the syntax and behavior of HTTP/1.1 messages, including requests, responses, and connections.
- [RFC 3875 — CGI Version 1.1](https://www.rfc-editor.org/rfc/rfc3875.html)
  Defines the CGI and how web servers communicate with it.

### Networking

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
  A practical guide to network programming and socket-based communication.

## AI Usage

AI was used as an additional resource during the project, mainly for understanding concepts, discussing general implementation, comparing to NGINX, improving text documentation and creating file generation scripts.
