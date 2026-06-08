# Webserv: C++98 Architecture & Implementation Strategy

Designing this server requires breaking the system down into distinct objects with strict single responsibilities. Because we are restricted to C++98, we must rely on standard containers (`std::vector`, `std::map`) and build robust manual parsing logic.

## 1. Core Class Responsibilities
This breakdown forms the mental model for your primary C++ classes.

| Class / Component | Responsibility & Logic Flow | Key C++98 Structures / Concepts |
| :--- | :--- | :--- |
| **ConfigParser** | Reads the `.conf` file, tokenizes it by handling whitespace and semicolons, and validates curly brace scope. It acts as a factory, producing a list of `ServerConfig` objects. If syntax is bad, it throws an exception before any sockets are opened. | `std::ifstream`, `std::string::find`, `std::vector<std::string>` for tokens. Needs recursive descent or a stack for bracket matching. |
| **ServerConfig** | A pure data object holding settings for one virtual server (ports, hostnames, default error pages). Contains a nested collection of `LocationBlock` objects to handle specific URI routes. | `std::string server_name`, `int port`, `std::map<int, std::string> error_pages`, `std::vector<LocationBlock>`. |
| **Engine / Core** | The heart of the program. Initializes listening sockets based on configurations, then enters an infinite `poll()` loop. It acts as the multiplexer, delegating read/write events to the appropriate `Client` object. | `std::vector<struct pollfd>`, `socket()`, `bind()`, `listen()`. Must handle `POLLIN`, `POLLOUT`, and `POLLHUP`. |
| **Client** | Represents a single connected user. It lives and dies by the connection. It stores the raw bytes received so far, tracks connection state, and owns an instance of an `HttpRequest` and an `HttpResponse`. | `int fd`, `time_t last_activity` (for timeouts), `std::vector<char> request_buffer`, `enum State { RECV_HDR, RECV_BODY, SEND_RES, DONE }`. |
| **HttpRequest** | A state machine that consumes raw bytes from the Client's buffer. It parses the Request Line first, then headers, then the body. It knows when a request is fully formed (e.g., reaching Content-Length). | `std::map<std::string, std::string> headers`. Chunked transfer decoding logic happens here. |
| **HttpResponse** | Takes the parsed `HttpRequest` and the `ServerConfig`, routes the URI, opens the required file (or triggers CGI), and constructs the raw HTTP response string to be queued for sending. | `std::string status_line`, `std::string headers`, `std::vector<char> body`. Uses `stat()` to check file/dir existence. |
| **CgiHandler** | Spawned when a route requires a script. Sets up bidirectional pipes, forks a child process, executes the script, and feeds it the POST body while reading its stdout. | `pipe()`, `fork()`, `execve()`, `dup2()`. Must map HTTP headers to CGI environment variables (`char** envp`). |

## 2. Foundational Utilities to Build First
Before writing the core loop, we need these tools to survive C++98 limitations:

### A. The String Manipulation Library
We will be doing a massive amount of string parsing (headers, config files, chunked bodies). Create a dedicated namespace or static class for string utilities:
* `trim()`: Remove leading/trailing whitespace (crucial for header values).
* `split()`: Break a string into a `std::vector<std::string>` by a delimiter.
* **Line Reader:** A safe way to extract substrings up to `
` without discarding the rest of the buffer.

### B. The Buffer State Machine
A single `recv()` might return 10 bytes of a 1000-byte header. We cannot assume we get a full request at once. Our `Client` class must append incoming bytes to a persistent buffer (e.g., `std::string` or `std::vector<char>`). The state machine checks the buffer: *Do I have yet? If no, wait for next `poll()`. If yes, parse headers and check if a body is expected."*

### C. Custom Exception Hierarchy
HTTP is prone to errors (400 Bad Request, 404 Not Found, 413 Payload Too Large). By using custom exceptions (e.g., `throw HttpException(404)`), we can abort processing a bad request deep within the parser, catch it at the `Client` level, and instantly generate the corresponding error page.

### D. The Timeout Manager
`poll()` will not notify you if a client opened a connection and then did nothing. Every time a client reads or writes, we must update a timestamp. Once per iteration of our main loop, we iterate through all active clients. If `current_time - client.last_activity > TIMEOUT_LIMIT`, close the socket and remove them. Otherwise, we will run out of file descriptors during a stress test.

## 3. The Non-Blocking Paradigm (Crucial)
Because we are using exactly one thread and one `poll()` loop, every socket must be non-blocking (using `fcntl(fd, F_SETFL, O_NONBLOCK)`). This changes how we read and write:

* **Reading:** We must call `recv()` in a loop until it returns `-1` with `errno == EAGAIN` or `EWOULDBLOCK`. This means *"I have read all available data for now, go back to `poll()`"*.
* **Writing:** If we have a 10MB response, calling `send()` might only send 64KB before returning. We must track how many bytes were sent, slice our buffer, and wait for the next `POLLOUT` event to send the rest. Never use a `while(bytes_sent < total)` loop inside our event handler, or the entire server will hang for other users.
* **CGI:** `waitpid()` will block our server if the CGI script enters an infinite loop. We must use `waitpid(pid, &status, WNOHANG)` to check on the script without pausing the server, or monitor the CGI's stdout pipe using `poll()`.
