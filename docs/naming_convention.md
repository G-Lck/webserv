# Naming Convention

## Goal
Keep names clear, predictable, and easy to read.

## 1) Class names: PascalCase
```cpp
class HttpRequestParser {
public:
    void parseRequestLine();
};
```

## 2) Function names: camelCase
```cpp
void parseHeaders();
int buildResponseCode();
```

## 3) Variable names: snake_case
```cpp
int client_fd = 0;
std::string request_path;
size_t content_length = 0;
```

## 4) Private members: prefix with _
```cpp
class ClientConnection {
private:
    int _socket_fd;
    std::string _remote_ip;
};
```

Comment on private member naming:
- Prefix `_member` is common and works in class scope.
- In C++, avoid reserved identifiers:
  - `__name` is reserved.
  - `_Name` (underscore + uppercase) is reserved.
- Alternative commonly used in modern C++: trailing underscore (`member_`).
- Choose one style for the whole project and stay consistent.

## 5) Avoid generic names
Bad:
```cpp
int data;
std::string value;
```

Better:
```cpp
int active_connections;
std::string raw_request;
```

## 6) Constants: UPPER_SNAKE_CASE
```cpp
static const int MAX_CLIENTS = 1024;
static const int DEFAULT_PORT = 8080;
```

## Canonical Form Reminder (42)
Orthodox Canonical Form:
- Default constructor
- Copy constructor
- Copy assignment operator
- Destructor

Reminder:
- It is NOT explicitly required everywhere in the subject.
- Implement it when it is useful or required by your class behavior.

## Namespace Reminder
Namespaces seem allowed, so we can use project namespaces.

Recommended:
```cpp
namespace ws {
class Server {};
}
```

About `std`:
- Avoid `using namespace std;` in headers.
- Prefer explicit names (`std::string`, `std::vector`) for clarity and safety.
