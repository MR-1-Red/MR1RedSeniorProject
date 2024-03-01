import socket
import ssl
import time
import oqs
from pprint import pprint

alg = "HQC-128"
hostname = "www.example.com" 
context = ssl.create_default_context()
s = socket.create_connection((hostname, 443))
wrapped_socket = context.wrap_socket(s, server_hostname=hostname)
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

        