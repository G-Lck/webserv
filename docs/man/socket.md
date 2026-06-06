# man: socket()

Allocate a socket descriptor.

## Synopsis

```cpp
#include <sys/types.h>
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
```

## Description

Returns a new socket descriptor that you can use to do sockety things with. This is generally the first call
in the whopping process of writing a socket program, and you can use the result for subsequent calls to
listen(), bind(), accept(), or a variety of other functions.

In usual usage, you get the values for these parameters from a call to getaddrinfo(), as shown in the
example below. But you can fill them in by hand if you really want to.

| Parameter | Description |
|---|---|
| domain | domain describes what kind of socket you're interested in. This can, believe me, be a wide variety of things, but since this is a socket guide, it's going to be PF_INET for IPv4, and PF_INET6 for IPv6. |
| type | Also, the type parameter can be a number of things, but you'll probably be setting it to either SOCK_STREAM for reliable TCP sockets (send(), recv()) or SOCK_DGRAM for unreliable fast UDP sockets (sendto(), recvfrom()). (Another interesting socket type is SOCK_RAW which can be used to construct packets by hand. It's pretty cool.) |
| protocol | Finally, the protocol parameter tells which protocol to use with a certain socket type. Like I've already said, for instance, SOCK_STREAM uses TCP. Fortunately for you, when using SOCK_STREAM or SOCK_DGRAM, you can just set the protocol to 0, and it'll use the proper protocol automatically. Otherwise, you can use getprotobyname() to look up the proper protocol number. |

## Return Value

The new socket descriptor to be used in subsequent calls, or -1 on error (and errno will be set accordingly).

## Example

```cpp
struct addrinfo hints, *res;
int sockfd;

// first, load up address structs with getaddrinfo():

memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC; // AF_INET, AF_INET6, or AF_UNSPEC
hints.ai_socktype = SOCK_STREAM; // SOCK_STREAM or SOCK_DGRAM

getaddrinfo("www.example.com", "3490", &hints, &res);

// make a socket using the information gleaned from getaddrinfo():
sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
```

