#!/bin/bash
trap "exit;" SIGINT
if ! [ -e handshakes.txt ]
then
    touch handshakes.txt
fi

if ! [ -e TLStimes.txt ]
then
    touch TLStimes.txt
fi

kem_algs=("kyber512" "kyber768" "kyber1024" "frodo640aes" "frodo976aes" "frodo1344aes" "bikel1" "bikel3" "bikel5" "hqc128" "hqc192" "hqc256" "mlkem512" "mlkem768" "mlkem1024")
kem_algs_short=("P-256")
for alg in "${kem_algs_short[@]}"
do 
    for i in {1..20}
    do
        #time (openssl s_client -groups $alg -CAfile rsa_CA.crt  >> handshakes.txt) >> TLStimes.txt 
        time (openssl s_client -groups $alg -CAfile rsa_CA.crt >> handshakes.txt) >> TLStimes.txt 2>&1 <<EOF
        Q
EOF
    done
done
