import socket
import ssl

hostname = "www.example.com"
context = ssl.create_default_context()

s = socket.create_connection((hostname, 443))
wrapped_socket=context.wrap_socket(s, server_hostname=hostname)
print(wrapped_socket.version())
        