#include <openssl/evp.h>
#include <openssl/provider.h>
#include <openssl/rand.h>
#include <string.h>

static OSSL_LIB_CTX *libctx = NULL;

int main(int argc, char const *argv[])
{
    EVP_PKEY *key = NULL;
    unsigned char *out = NULL;
    unsigned char secenc[32];
    RAND_bytes(secenc, 32);
    unsigned char secdec[32];
    size_t outlen, seclen;

    libctx=OSSL_LIB_CTX_new();
    OSSL_PROVIDER_load(libctx, "oqsprovider");
    OSSL_PROVIDER_load(libctx, "default");
    EVP_PKEY_CTX *keyctx = EVP_PKEY_CTX_new_from_name(libctx, "hqc256", NULL);
    //clock_t genbegin = clock();
    EVP_PKEY_keygen_init(keyctx);
    EVP_PKEY_generate(keyctx, &key);
    //clock_t genend = clock();

        //ERR_print_errors_fp(stderr);
        //exit(EXIT_FAILURE);
    

    EVP_PKEY_encapsulate_init(keyctx, NULL);
    EVP_PKEY_encapsulate(keyctx, NULL, &outlen, NULL, &seclen);
    EVP_PKEY_encapsulate(keyctx, out, &outlen, secenc, &seclen);
    EVP_PKEY_decapsulate_init(keyctx, NULL);
    EVP_PKEY_decapsulate(keyctx, secdec, &seclen, out, outlen);
    if (memcmp(secdec,secenc,seclen)==0)
    {
        printf("success");
    }else{
        printf("did not match");
    }

    

      

   


    //EVP_PKEY_free(key);
    //EVP_PKEY_CTX_free(keyctx);
    //OPENSSL_free(out);
    //OPENSSL_free(secenc);
    //OPENSSL_free(secdec);
    //OSSL_LIB_CTX_free(libctx);

    return 0;

}
