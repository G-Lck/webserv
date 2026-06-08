# Webserv: Foundation Practice Exercises

To master the core architecture, we need to isolate the system calls before integrating them into our main project. Here are three progressive exercises designed to help us understand the mechanics of sockets and multiplexing in C++98.

## Exercise 1: The Echo Chamber (Blocking Sockets)
**Subject:** Create a simple server that accepts a single connection, reads the incoming data, prints it to the terminal, and sends a valid HTTP 200 response back before closing the connection.

**Guidelines:**
1. Use `socket()`, `bind()`, and `listen()` to open port `8080`.
2. Use `accept()` to wait for a client. (Our program will freeze/block here until someone connects).
3. Once connected, use `recv()` to read the incoming data into a `char` buffer (e.g., 1024 bytes).
4. Print the buffer to the standard output so we can see the raw HTTP request.
5. Use `send()` to transmit a hardcoded string: `"HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello Server!"`.
6. Close the client socket and the server socket.
7. **Test it:** Open a web browser and navigate to `http://localhost:8080`.

**Questions to guide our reasoning:**
* What happens if the browser sends more than 1024 bytes? Does our program crash, or does it just leave data in the OS buffer?
* What exact integer value does `recv()` return when the client disconnects unexpectedly?

---

## Exercise 2: The Juggler (Non-Blocking & poll)
**Subject:** Modify the server to handle multiple clients simultaneously without ever freezing. If client A connects and does nothing, client B must still be able to connect and communicate.

**Guidelines:**
1. Set the main listening socket to non-blocking using `fcntl(fd, F_SETFL, O_NONBLOCK)`.
2. Create an array or `std::vector` of `struct pollfd`. Add the listening socket to it, watching for the `POLLIN` event.
3. Enter an infinite loop, calling `poll()`.
4. Iterate through the `pollfd` structures. If `poll()` indicates the listening socket has `POLLIN`, call `accept()`. Set the newly accepted client socket to non-blocking, and add it to our `pollfd` vector.
5. If `poll()` indicates a client socket has `POLLIN`, call `recv()`. If `recv()` returns data, print it. If it returns `0` (client disconnected), close the socket and erase it from our vector.
6. **Test it:** Open two separate terminal windows. Use `telnet localhost 8080` in both. Type messages in both terminals. Neither terminal should freeze the other.

**Questions to guide our reasoning:**
* Why do we check the `poll()` event flags before calling `accept()` or `recv()` now?
* Because our sockets are non-blocking, if `recv()` returns `-1`, what does the `errno` variable tell us?

---

## Exercise 3: The Drip Feeder (State Machines & Buffering)
**Subject:** Simulate a terrible, slow network connection. We must safely read an HTTP request even if `recv()` only gives us a few bytes at a time, without blocking other clients.

**Guidelines:**
1. Build upon the code from Exercise 2.
2. Instead of using a temporary local `char buffer[1024]`, give each connected client its own persistent `std::string` or `std::vector<char>` buffer (perhaps by creating a simple `Client` struct/class).
3. Force our `recv()` function to read a maximum of **5 bytes** per call. This simulates heavy network fragmentation.
4. Append those 5 bytes to that specific client's persistent buffer.
5. After every read, search the client's buffer for the string `\r\n\r\n` (the end of an HTTP header).
6. Only when `\r\n\r\n` is found, print `"FULL REQUEST HEADER RECEIVED!"` and print the complete buffer.

**Questions to guide our reasoning:**
* How does giving each client their own dedicated buffer prevent data corruption when multiple clients are sending data simultaneously?
* If a client sends a 20-byte request, how many times will our main `poll()` loop iterate to read the full message?
