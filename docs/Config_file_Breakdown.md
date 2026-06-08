# Webserv Configuration Guide: Understanding Our Blueprint

Here is the breakdown of the configuration file we are working with. To make it easier for us to visualize, let's think of the `server` block as the rules for a specific **Office Building**, and the `location` blocks as the rules for specific **Rooms** inside that building.

# Webserv Configuration File Example

Here is the reference configuration file we are using for our `webserv` project. We will use this to test our `ConfigParser` class and ensure our routing logic correctly maps incoming requests to the right locations.

```nginx
server {
    listen 8002;
    server_name localhost;
    host 127.0.0.1;
    root docs/fusion_web/;
    # client_max_body_size 3000000;
    index index.html;
    error_page 404 error_pages/404.html;

    location / {
        allow_methods  DELETE POST GET;
        autoindex off;
    }
    
    location /tours {
        autoindex on;
        index tours1.html;
        allow_methods GET POST PUT HEAD;
    }

    location /red {
        return /tours;
    }

    location /cgi-bin {
        root ./;
        allow_methods GET POST DELETE;
        index time.py;
        cgi_path /usr/bin/python3 /bin/bash;
        cgi_ext .py .sh;
    }
}
```


## 1. The Main Server Configuration (The Building)

| Directive | What it means technically for us | The Easy Example |
| :--- | :--- | :--- |
| `listen 8002;` | The TCP port our `poll()` loop will monitor. | **The Door Number:** Clients must explicitly go to Door 8002 to enter our building. |
| `host 127.0.0.1;` | The IP address we will `bind()` the socket to. | **The Street Address:** The physical location of our server. |
| `server_name localhost;` | The domain name we match against the HTTP `Host:` header. | **The Company Sign:** If multiple companies share Door 8002, this checks if the visitor actually wants us. |
| `root docs/fusion_web/;` | The base directory on our hard drive where we look for files. | **The Basement Storage:** If a user asks for `/image.png`, we physically look in `docs/fusion_web/image.png`. |
| `# client_max_body_size` | Limits the maximum size of a POST request body we accept. | **The Mail Slot Size:** Packages larger than this get rejected by us at the door with a `413 Payload Too Large` error. |
| `index index.html;` | The default file we serve if a client requests a directory. | **The Receptionist:** If visitors enter without asking for a specific person, `index.html` greets them. |
| `error_page 404 ...` | Maps HTTP error codes to our custom HTML files. | **The Apology Poster:** If we can't find a file, we show a branded "Oops!" page instead of a blank screen. |

## 2. The Routing Rules (The Rooms)

When an HTTP request comes in (e.g., `GET /tours/guide.pdf`), our C++ router must find the longest matching `location` block to decide how we handle it.

| Location Block | Security & Rules | What happens when a user visits |
| :--- | :--- | :--- |
| `location /` <br>*(The Default Lobby)* | `allow_methods DELETE POST GET;`<br>`autoindex off;` | This is our catch-all for any URI that doesn't match a more specific room. <br><br>**Example:** Visiting `http://127.0.0.1:8002/`.<br>Because `autoindex` is off, if we don't have an `index.html` in the root, we throw a `403 Forbidden` (like a locked door with no window). |
| `location /tours` <br>*(The Travel Dept)* | `autoindex on;`<br>`index tours1.html;`<br>`allow_methods GET POST PUT HEAD;` | Any URL starting with `/tours`. <br><br>**Example:** If the user goes to `/tours/` and `tours1.html` is missing, our `autoindex on` kicks in. We must dynamically generate an HTML page listing all the files in that directory (like looking through a glass window into the room). |
| `location /red` <br>*(The Redirect Room)* | `return /tours;` | This triggers an HTTP Redirection. <br><br>**Example:** If a user requests `/red`, we immediately reply with a `301` or `302` status code and a `Location: /tours` header. The user's browser will automatically jump to `/tours`. |
| `location /cgi-bin` <br>*(The Factory)* | `root ./;`<br>`index time.py;`<br>`cgi_ext .py .sh;`<br>`cgi_path ...` | This room executes code instead of just reading static files.<br><br>**Example:** If a user requests `/cgi-bin/time.py`, we see the `.py` extension, check our `cgi_path`, and use `fork()` and `execve()` to run `/usr/bin/python3` on that script. We intercept the script's output via a `pipe()` and send it back to the client. |

### How we should think about this in our architecture:
When we write our `ConfigParser` class, we will read this text and populate a `ServerConfig` C++ object. This object will contain variables like `int port = 8002` and a `std::vector<LocationBlock>` representing the different rooms we need to manage.
