# Webserv: A Prototyping Approach to Architecture

When tackling a massive system from scratch, the worst thing we can do is start writing the final architecture immediately. We need to isolate the unknowns, build throwaway prototypes to understand the underlying mechanics, and only then design the main system.

## Implementation Phases

### Phase 1: The Connection
* **Focus Area:** Raw Sockets
* **Actionable Prototype:** Write a 50-line program using `socket()`, `bind()`, `listen()`, and `accept()`. Connect to it with a browser and print what `recv()` gets.
* **The Reasoning:** Demystifies how the OS hands we network traffic. we will see the raw text a browser actually sends.

### Phase 2: The Protocol
* **Focus Area:** HTTP Basics
* **Actionable Prototype:** Modify our prototype to `send()` a hardcoded `HTTP/1.1 200 OK\r\n\r\nHello World` back to the browser.
* **The Reasoning:** Proves that a web server is just a program that reads and writes formatted strings.

### Phase 3: The Engine
* **Focus Area:** Multiplexing
* **Actionable Prototype:** Rewrite the prototype to handle two simultaneous clients using `poll()`, making the sockets non-blocking.
* **The Reasoning:** This is the core constraint of the system. If we don't master `poll()` early, the final architecture will collapse under stress.

### Phase 4: The Blueprint
* **Focus Area:** Architecture
* **Actionable Prototype:** Map out our classes on a whiteboard. Define how `Client`, `Request`, and `Server` interact around the `poll()` loop.
* **The Reasoning:** We must decide how to store incomplete data chunks from non-blocking sockets before writing the real code.
