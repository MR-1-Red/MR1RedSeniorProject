/*
 *  Copyright 2022-2023 The OpenSSL Project Authors. All Rights Reserved.
 *
 *  Licensed under the Apache License 2.0 (the "License").  You may not use
 *  this file except in compliance with the License.  You can obtain a copy
 *  in the file LICENSE in the source distribution or at
 *  https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/provider.h>
#include <openssl/x509.h>
#include <signal.h>

static const int server_port = 4433;
static OSSL_LIB_CTX *libctx = NULL;

typedef unsigned char   bool;
#define true            1
#define false           0

/*
 * This flag won't be useful until both accept/read (TCP & SSL) methods
 * can be called with a timeout. TBD.
 */
static volatile bool    server_running = true;

int create_socket(bool isServer)
{
    int s;
    int optval = 1;
    struct sockaddr_in addr;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    if (isServer) {
        addr.sin_family = AF_INET;
        addr.sin_port = htons(server_port);
        addr.sin_addr.s_addr = INADDR_ANY;

        /* Reuse the address; good for quick restarts */
        if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))
                < 0) {
            perror("setsockopt(SO_REUSEADDR) failed");
            exit(EXIT_FAILURE);
        }

        if (bind(s, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
            perror("Unable to bind");
            exit(EXIT_FAILURE);
        }

        if (listen(s, 1) < 0) {
            perror("Unable to listen");
            exit(EXIT_FAILURE);
        }
    }

    return s;
}

SSL_CTX* create_context(bool isServer)
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    if (isServer)
        method = TLS_server_method();
    else
        method = TLS_client_method();
    libctx=OSSL_LIB_CTX_new();
    OSSL_PROVIDER_load(libctx, "oqsprovider");
    OSSL_PROVIDER_load(libctx, "default");
    ctx = SSL_CTX_new_ex(libctx, NULL, method);
    if (ctx == NULL) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    libctx=NULL;
    return ctx;
}


//old code. All key generation needs to be done in methods that use the key.

/*void generate_key(EVP_PKEY *keyloc){
    EVP_PKEY_CTX *keyctx = EVP_PKEY_CTX_new_from_name(libctx, "hqc256", NULL);
    libctx=OSSL_LIB_CTX_new();
    OSSL_PROVIDER_load(libctx, "oqsprovider");
    EVP_PKEY_generate(keyctx, &keyloc);
    //printf("key generated");
    //fflush(stdout);
    //EVP_PKEY_CTX_free(keyctx);
}*/

void configure_server_context(SSL_CTX *ctx)
{
    /* Set the key and cert */
    /*if (SSL_CTX_use_certificate_chain_file(ctx, "cert.pem") <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }*/
    libctx=OSSL_LIB_CTX_new();
    OSSL_PROVIDER_load(libctx, "oqsprovider");
    OSSL_PROVIDER_load(libctx, "default");
    EVP_PKEY *keyloc = NULL;
    EVP_PKEY_CTX *keyctx = EVP_PKEY_CTX_new_from_name(libctx, "falcon512", NULL);
    EVP_PKEY_keygen_init(keyctx);
    if (EVP_PKEY_generate(keyctx, &keyloc)<= 0){
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    EVP_PKEY_CTX_free(keyctx);
    keyctx=NULL;
    
    keyctx=EVP_PKEY_CTX_new_from_pkey(libctx, keyloc, NULL);
    
    //Need to change this
    //cert declaration
    X509 *x509;
    x509 = X509_new();
    //time to live and providing key
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
    X509_gmtime_adj(X509_getm_notBefore(x509), 0);
    X509_gmtime_adj(X509_getm_notAfter(x509), 31536000L);
    X509_set_pubkey(x509, keyloc);

    //other cert information
    X509_NAME *name;
    name = X509_get_subject_name(x509);
    X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC,
                           (unsigned char *)"CH", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                           (unsigned char *)"localhost", -1, -1, 0);
    X509_set_issuer_name(x509, name);
    if(X509_sign(x509, keyloc, EVP_sha1())<=0){
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    /*FILE * f;
    f = fopen("cert2.crt", "wb");
    PEM_write_X509(
    f,   
    x509  
    );*/
    /*if (SSL_CTX_use_certificate(ctx, x509)<=0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    //SSL_CTX_use_certificate();
    if (SSL_CTX_use_PrivateKey(ctx, keyloc) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    */
       if (SSL_CTX_use_certificate_chain_file(ctx, "rsa_srv.crt") <=0){
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, "rsa_srv.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}


//We shouldn't use this in the KEM experiments as we are unable to sign certificate while using pqc algorithms
void configure_client_context(SSL_CTX *ctx)
{
    /*
     * Configure the client to abort the handshake if certificate verification
     * fails
     */
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    /*
     * In a real application you would probably just use the default system certificate trust store and call:
     *     SSL_CTX_set_default_verify_paths(ctx);
     * In this demo though we are using a self-signed certificate, so the client must trust it directly.
     */
    if (!SSL_CTX_load_verify_locations(ctx, "rsa_srv.crt", NULL)) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

void usage(void)
{
    printf("Usage: sslecho s\n");
    printf("       --or--\n");
    printf("       sslecho c ip\n");
    printf("       c=client, s=server, ip=dotted ip of server\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    bool isServer;
    int result;

    SSL_CTX *ssl_ctx = NULL;
    SSL *ssl = NULL;

    int server_skt = -1;
    int client_skt = -1;

    /* used by getline relying on realloc, can't be statically allocated */
    char *txbuf = NULL;
    size_t txcap = 0;
    int txlen;

    char rxbuf[128];
    size_t rxcap = sizeof(rxbuf);
    int rxlen;

    char *rem_server_ip = NULL;

    struct sockaddr_in addr;
    unsigned int addr_len = sizeof(addr);

    /* ignore SIGPIPE so that server can continue running when client pipe closes abruptly */
    signal(SIGPIPE, SIG_IGN);

    /* Splash */
    printf("\nsslecho : Simple Echo Client/Server : %s : %s\n\n", __DATE__,
    __TIME__);

    /* Need to know if client or server */
    if (argc < 2) {
        usage();
        /* NOTREACHED */
    }
    isServer = (argv[1][0] == 's') ? true : false;
    /* If client get remote server address (could be 127.0.0.1) */
    if (!isServer) {
        if (argc != 3) {
            usage();
            /* NOTREACHED */
        }
        rem_server_ip = argv[2];
    }

    /* Create context used by both client and server */
    ssl_ctx = create_context(isServer);

    /* If server */
    if (isServer) {

        printf("We are the server on port: %d\n\n", server_port);

        /* Configure server context with appropriate key files */
        configure_server_context(ssl_ctx);

        /* Create server socket; will bind with server port and listen */
        server_skt = create_socket(true);

        /*
         * Loop to accept clients.
         * Need to implement timeouts on TCP & SSL connect/read functions
         * before we can catch a CTRL-C and kill the server.
         */
        while (server_running) {
            /* Wait for TCP connection from client */
            client_skt = accept(server_skt, (struct sockaddr*) &addr,
                    &addr_len);
            if (client_skt < 0) {
                perror("Unable to accept");
                exit(EXIT_FAILURE);
            }

            printf("Client TCP connection accepted\n");

            /* Create server SSL structure using newly accepted client socket */
            ssl = SSL_new(ssl_ctx);
            SSL_set_fd(ssl, client_skt);

            /* Wait for SSL connection from the client */
            if (SSL_accept(ssl) <= 0) {
                ERR_print_errors_fp(stderr);
                server_running = false;
            } else {

                printf("Client SSL connection accepted\n\n");

                /* Echo loop */
                while (true) {
                    /* Get message from client; will fail if client closes connection */
                    if ((rxlen = SSL_read(ssl, rxbuf, rxcap)) <= 0) {
                        if (rxlen == 0) {
                            printf("Client closed connection\n");
                        } else {
                            printf("SSL_read returned %d\n", rxlen);
                        }
                        ERR_print_errors_fp(stderr);
                        break;
                    }
                    /* Insure null terminated input */
                    rxbuf[rxlen] = 0;
                    /* Look for kill switch */
                    if (strcmp(rxbuf, "kill\n") == 0) {
                        /* Terminate...with extreme prejudice */
                        printf("Server received 'kill' command\n");
                        server_running = false;
                        break;
                    }
                    /* Show received message */
                    printf("Received: %s", rxbuf);
                    /* Echo it back */
                    if (SSL_write(ssl, rxbuf, rxlen) <= 0) {
                        ERR_print_errors_fp(stderr);
                    }
                }
            }
            if (server_running) {
                /* Cleanup for next client */
                SSL_shutdown(ssl);
                SSL_free(ssl);
                close(client_skt);
            }
        }
        printf("Server exiting...\n");
    }
    /* Else client */
    else {

        printf("We are the client\n\n");

        /* Configure client context so we verify the server correctly */
        //uneeded with KEMTLS
        //configure_client_context(ssl_ctx);

        /* Create "bare" socket */
        client_skt = create_socket(false);
        /* Set up connect address */
        addr.sin_family = AF_INET;
        inet_pton(AF_INET, rem_server_ip, &addr.sin_addr.s_addr);
        addr.sin_port = htons(server_port);
        /* Do TCP connect with server */
        if (connect(client_skt, (struct sockaddr*) &addr, sizeof(addr)) != 0) {
            perror("Unable to TCP connect to server");
            goto exit;
        } else {
            printf("TCP connection to server successful\n");
        }

        /* Create client SSL structure using dedicated client socket */
        ssl = SSL_new(ssl_ctx);
        SSL_set_fd(ssl, client_skt);
        /* Set hostname for SNI */
        SSL_set_tlsext_host_name(ssl, rem_server_ip);
        /* Configure server hostname check */
        SSL_set1_host(ssl, rem_server_ip);

        /* Now do SSL connect with server */
        if (SSL_connect(ssl) == 1) {

            printf("SSL connection to server successful\n\n");

            /* Loop to send input from keyboard */
            while (true) {
                /* Get a line of input */
                txlen = getline(&txbuf, &txcap, stdin);
                /* Exit loop on error */
                if (txlen < 0 || txbuf == NULL) {
                    break;
                }
                /* Exit loop if just a carriage return */
                if (txbuf[0] == '\n') {
                    break;
                }
                /* Send it to the server */
                if ((result = SSL_write(ssl, txbuf, txlen)) <= 0) {
                    printf("Server closed connection\n");
                    ERR_print_errors_fp(stderr);
                    break;
                }

                /* Wait for the echo */
                rxlen = SSL_read(ssl, rxbuf, rxcap);
                if (rxlen <= 0) {
                    printf("Server closed connection\n");
                    ERR_print_errors_fp(stderr);
                    break;
                } else {
                    /* Show it */
                    rxbuf[rxlen] = 0;
                    printf("Received: %s", rxbuf);
                }
            }
            printf("Client exiting...\n");
        } else {

            printf("SSL connection to server failed\n\n");

            ERR_print_errors_fp(stderr);
        }
    }
exit:
    /* Close up */
    if (ssl != NULL) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    SSL_CTX_free(ssl_ctx);

    if (client_skt != -1)
        close(client_skt);
    if (server_skt != -1)
        close(server_skt);

    if (txbuf != NULL && txcap > 0)
        free(txbuf);

    printf("sslecho exiting\n");

    return EXIT_SUCCESS;
}