import socket
#import oqs
import time
import threading

HEADER = 64
PORT = 443
FORMAT = 'utf-8'
SERVER = '0.0.0.0' #socket.gethostbyname(socket.gethostname())
ADDR = (SERVER, PORT)
END = 'closed'

print(socket.gethostname())
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(ADDR)

def start():
    print("listening")
    server.listen()
    while True:
        conn, addr = server.accept()
        thread = threading.Thread(target=receive_conn, args=(conn, addr))
        thread.start()
        
        
def receive_conn(conn, addr):
    print(f"connection established with {addr}")
    
    connected=True
    while connected:
        #msg_length = int(conn.recv(HEADER).decode(FORMAT))
        msg = conn.recv(1024).decode(FORMAT)
        if msg=="close":
            conn.send("closed".encode(FORMAT))
            connected=False
        print(msg)
    conn.close()
    
start()