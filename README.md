Generating certificates for handshake test:

openssl req -x509 -new -newkey rsa:4096 -keyout rsa_CA.key -out rsa_CA.crt -nodes -subj "/CN=test CA" -days 365 
openssl genpkey -algorithm rsa:4096 -out rsa_srv.key
openssl req -new -newkey rsa:4096 -keyout rsa_srv.key -out rsa_srv.csr -nodes -subj "/CN=test server" 
openssl x509 -req -in rsa_srv.csr -out rsa_srv.crt -CA rsa_CA.crt -CAkey rsa_CA.key -CAcreateserial -days 365

Running handshake test:


Sever:

openssl s_server -cert rsa_srv.crt -key rsa_srv.key -www -tls1_3 -groups kyber512:kyber768:kyber1024:frodo640aes:frodo976aes:frodo1344aes:bikel1:bikel3:bikel5:hqc128:hqc192:hqc256:mlkem512:mlkem768:mlkem1024

Client:

openssl s_client -groups frodo640aes -CAfile rsa_CA.crt


These should be run in two seperate terminals. Have wireshark monitoring loopback to get packets. raplace "frodo640aes" with whatever algorithm you want to test or run the TLStest.sh script to test all algorithms


