import socket
import ssl
import time
import oqs
from pprint import pprint

alg = "HQC-128"
hostname = "www.example.com" 

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

server_port = 443 #Server port number
client.connect(('localhost', server_port)) #Change first variable to the server_ip
while True:
    # input message and send it to the server
    msg = input("This is client: ")
    client.send(msg.encode("utf-8")[:1024])

    response = client.recv(1024)
    response = response.decode("utf-8")

    # if server sent us "closed" in the payload, we; break out of the loop and close our socket
    if response.lower() == "closed":
        break

    print(f"Received: {response}")

client.close()
print("Connection to server closed")
#print(wrapped_socket.version())
#wrapped_socket.send("GET / HTTP/ ")

kems = oqs.get_enabled_kem_mechanisms()

# create client and server with sample KEM mechanisms
with oqs.KeyEncapsulation(alg) as client:
    with oqs.KeyEncapsulation(alg) as server:
        print("\nKey encapsulation details:")
        pprint(client.details)

        # client generates its keypair
        genStart=time.time()
        public_key_client = client.generate_keypair()
        genEnd=time.time()
        total = genEnd-genStart
        print("Generation Time: "+str(total)+"s")
        
        # the server encapsulates its secret using the client's public key
        enStart=time.time()
        ciphertext, shared_secret_server = server.encap_secret(public_key_client)
        enEnd =time.time()
        total = enEnd-enStart
        print("Encryption time: "+str(total)+"s")
        
        # the client decapsulates the server's ciphertext to obtain the shared secret
        deStart=time.time()
        shared_secret_client = client.decap_secret(ciphertext)
        deEnd=time.time()
        total = deEnd-deStart
        print("Decryption time: "+str(total)+"s")
        print("\nShared secretes coincide:", shared_secret_client == shared_secret_server)

        