# Webserv: Configuration Parsing Strategy

To build our parser, we need to understand the structural rules of the Nginx format we are replicating. It is a hierarchical, token-based syntax. Here is the breakdown of the format we need to handle in our C++ code:

## 1. Format Breakdown & Parsing Goals

| Format Element | Syntax Rule | Our C++ Parsing Goal |
| :--- | :--- | :--- |
| **Tokens** | Words separated by spaces, tabs, or newlines. | Split the raw file string into a `std::vector<std::string>` of readable words. |
| **Simple Directives** | `key value1 value2;`<br>Must end with a semicolon. | Match the key to a variable, save the values, and throw an exception if the `;` is missing. |
| **Block Directives** | `name [optional_params] { ... }`<br>Containers for other directives. | Detect `{` to open a new scope (like a `Server` or `Location` object) and `}` to close it. |
| **Contexts (Scope)** | Blocks nested inside other blocks. | Implement inheritance. If a `location` doesn't have a `root` directive, we must fall back to the `root` defined in the server block. |
| **Comments** | `# anything after this is ignored` | Find `#` characters and completely erase everything up to the `\n` before we start tokenizing. |

---

## 2. Expected Directives & Storage

We can visualize the keys and values we need to handle based on our project requirements:

### Server Context Directives
These are the keys we will find directly inside the `server { ... }` block.

| Key | Expected Value(s) | Example | C++ Storage Type Idea |
| :--- | :--- | :--- | :--- |
| `listen` | A port number (or `IP:Port`). | `8002;` | `int` |
| `host` | An IP address. | `127.0.0.1;` | `std::string` |
| `server_name` | A domain name. | `localhost;` | `std::string` |
| `root` | A directory path. | `docs/fusion_web/;` | `std::string` |
| `client_max_body_size` | A number representing bytes. | `3000000;` | `size_t` |
| `index` | The default HTML filename. | `index.html;` | `std::string` |
| `error_page` | An HTTP error code AND a path. | `404 error_pages/404.html;` | `std::map<int, std::string>` |

### Location Context Directives
These are the keys we will find inside a `location /path { ... }` block. Notice that a Location block can also contain `root` and `index`, which should *override* the server's default settings.

| Key | Expected Value(s) | Example | C++ Storage Type Idea |
| :--- | :--- | :--- | :--- |
| `allow_methods` | One or more HTTP methods. | `GET POST DELETE;` | `std::vector<std::string>` |
| `autoindex` | Strictly `on` or `off`. | `on;` | `bool` |
| `return` | A path to redirect to. | `/tours;` | `std::string` |
| `cgi_ext` | One or more file extensions. | `.py .sh;` | `std::vector<std::string>` |
| `cgi_path` | Paths to the executing binaries. | `/usr/bin/python3 /bin/bash;` | `std::vector<std::string>` |

---

## 3. How the Logic Flows (The Recursive Descent Idea)

If our vector of tokens looks like this:
`["server", "{", "listen", "8002", ";", "location", "/", "{", "autoindex", "off", ";", "}", "}"]`

We just use a single `size_t i = 0` to iterate through the vector:
1. `tokens[i] == "server"` -> We create a new `ServerConfig` object.
2. We see `{`, so we enter a loop to parse server keys.
3. We see `listen`, so we grab the next token (`8002`) and save it.
4. We see `location`, so we create a new `LocationBlock` object. We call a function like `parseLocation(tokens, i)` which takes over moving `i` forward until it hits the closing `}`.
5. Once `parseLocation` finishes, we are back in our server loop, continuing where we left off.

---

## 4. Key Ordering and Conflict Resolution

We don’t need to enforce a strict key order. In real Nginx configurations, simple directives inside a block can be organized completely randomly. A user can put `root` before `listen`, or `error_page` at the very end, and the server must still understand it.

Here is how we should reason about ordering as we build our parser:

| Configuration Element | Does Order Matter? | How we handle it in C++98 |
| :--- | :--- | :--- |
| **Simple Keys**<br>*(e.g., `listen`, `root`, `server_name`)* | **No.** They can be completely random. | As we loop through our tokens, we just fill our `ServerConfig` variables or `std::map`. Because we parse everything before opening any sockets, the order in which we collect the data doesn't impact the engine. |
| **Duplicate Keys**<br>*(e.g., two `root` directives in one block)* | **Yes.** (For conflict resolution) | We need to decide our rule. If the user writes `root /a;` and then `root /b;`, do we throw an exception, or do we let the second one overwrite the first? *(Nginx usually throws an error for duplicates like `listen`, but overwrites others)*. |
| **`location` Blocks** | **It depends on our matching logic.** | When a request comes in for `/tours/images/`, our router has to find the right room. If we implement "longest-prefix matching" (finding the most specific path), the order of blocks in the file doesn't matter. If we just iterate through our `std::vector<LocationBlock>` and stop at the first partial match, order *would* matter. We should aim for longest-prefix matching. |