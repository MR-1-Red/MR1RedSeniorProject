
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

int main(int argc, char const *argv[])
{
    static OSSL_LIB_CTX *libctx = NULL;
    libctx=OSSL_LIB_CTX_new();
    OSSL_PROVIDER_load(libctx, "oqsprovider");
    OSSL_PROVIDER_load(libctx, "default");
    if (OSSL_PROVIDER_available(libctx, "oqsprovider")<=0){
        perror("oqsprovider not found");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    const SSL_METHOD *method;
    SSL_CTX *ctx;
    method = TLS_client_method();
    ctx = SSL_CTX_new_ex(libctx, NULL, method);
    SSL_CTX_set1_groups_list(ctx, "dilithium3");
    if (ctx == NULL) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    
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

    if (SSL_CTX_use_certificate_chain_file(ctx, "qsc.crt") <=0){
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, "qsc.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return 0;
}
