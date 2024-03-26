if you need to generate keys for non-pqc test server 
openssl req -newkey rsa:4096 -x509 -sha256 -days 3650 -nodes -out cert.pem -keyout key.pem
