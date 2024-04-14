#include <stdio.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/provider.h>
#include <openssl/rand.h>
#include <openssl/params.h>
#include <string.h>

static OSSL_LIB_CTX *libctx = NULL;

int main(int argc, char const *argv[])
{
    FILE *fp = fopen("Output.txt", "w");// "w" means that we are going to write on this file
    for (size_t i = 0; i < 20; i++)
    {
    
    
    EVP_PKEY *key = NULL;
    unsigned char *out=NULL;
    unsigned char *secenc=NULL;
    unsigned char *secdec=NULL;
    size_t outlen, seclen;
    OSSL_PARAM params[2];

    libctx=OSSL_LIB_CTX_new();
    OSSL_PROVIDER_load(libctx, "oqsprovider");
    OSSL_PROVIDER_load(libctx, "default");
    EVP_PKEY_CTX *keyctx = EVP_PKEY_CTX_new_from_name(libctx, "EC", NULL);
    EVP_PKEY_keygen_init(keyctx);
    params[0]= OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME, "P-256", 0);
    params[1]=OSSL_PARAM_construct_end();
    clock_t genbegin = clock();
    
    EVP_PKEY_generate(keyctx, &key);
    clock_t genend = clock();
    double genticks = (double)(genend - genbegin);
    double gentime= (double)genticks / CLOCKS_PER_SEC;
        //ERR_print_errors_fp(stderr);
        //exit(EXIT_FAILURE);
    EVP_PKEY_CTX_free(keyctx);
    keyctx=NULL;
    keyctx=EVP_PKEY_CTX_new_from_pkey(libctx, key, NULL);

    EVP_PKEY_encapsulate_init(keyctx, NULL);
    EVP_PKEY_encapsulate(keyctx, NULL, &outlen, NULL, &seclen);
    out = OPENSSL_malloc(outlen);
    secenc = OPENSSL_malloc(seclen);
    //RAND_bytes(secenc, 32);
    secdec = OPENSSL_malloc(seclen);
    clock_t encapbegin = clock();
    EVP_PKEY_encapsulate(keyctx, out, &outlen, secenc, &seclen);
    clock_t encapend = clock();
    double encapticks = (double)(encapend - encapbegin);
    double encaptime = (double)encapticks / CLOCKS_PER_SEC;

    clock_t decapbegin = clock();
    EVP_PKEY_decapsulate_init(keyctx, NULL);
    EVP_PKEY_decapsulate(keyctx, secdec, &seclen, out, outlen);
    clock_t decapend = clock();
    double decapticks = (double)(decapend - decapbegin);
    double decaptime = (double)decapticks / CLOCKS_PER_SEC;

    if (memcmp(secdec,secenc,seclen)==0)
    {
        printf("success");
        printf(" %ld\n", seclen);
        printf(" %ld\n", outlen);
    }else{
        printf("did not match");
        printf(" %ld\n", seclen);
        printf(" %ld\n", outlen);
    }

    //data in format "category, clockcycle, timetaken(s)"
    fprintf(fp, "G, %lf, %lf\n", genticks, gentime);
    fprintf(fp, "E, %lf, %lf\n", encapticks, encaptime);
    fprintf(fp, "D, %lf, %lf\n", decapticks, decaptime);
    OSSL_LIB_CTX_free(libctx);
    }
    fclose(fp); //Don't forget to close the file when finished
    
    /*EVP_PKEY_free(key);*/
    //EVP_PKEY_CTX_free(keyctx);
    //OPENSSL_free(out);
    //OPENSSL_free(secenc);
    //PENSSL_free(secdec);
    

    return 0;

}
