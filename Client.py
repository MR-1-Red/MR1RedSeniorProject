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
pprint(kems, compact=True)
# create client and server with sample KEM mechanisms
with oqs.KeyEncapsulation(alg) as client:
    with oqs.KeyEncapsulation(alg) as server:
        print("\nKey encapsulation details:")
        pprint(client.details)

        # client generates its keypair
        public_key_client = client.generate_keypair()
        

        # the server encapsulates its secret using the client's public key
        ciphertext, shared_secret_server = server.encap_secret(public_key_client)

        # the client decapsulates the server's ciphertext to obtain the shared secret
        shared_secret_client = client.decap_secret(ciphertext)

        print("\nShared secretes coincide:", shared_secret_client == shared_secret_server)

        