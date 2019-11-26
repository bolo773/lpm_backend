#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <iostream>
#include <fstream>

int decrypt(unsigned char * ciphertext, int ciphertext_len, unsigned char * key, unsigned char *iv, unsigned char * plaintext)
{
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;
    ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx,EVP_aes_256_cbc(),NULL,key,iv);
    EVP_DecryptUpdate(ctx,plaintext,&len,ciphertext,ciphertext_len);
    plaintext_len = len;
    EVP_DecryptFinal_ex(ctx,plaintext + len, &len);
    plaintext_len += len;
    EVP_CIPHER_CTX_free(ctx);
    return plaintext_len;
  
    
}

int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext){
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;
    ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx,EVP_aes_256_cbc(),NULL,key,iv);
    EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len);
    ciphertext_len = len;
    EVP_EncryptFinal_ex(ctx, ciphertext + len,&len);
    ciphertext_len += len;
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;

}

int main(void)
{
    
    std::ifstream image("enc_new.png", std::ios::binary | std::ios::ate);
    image.seekg(0,image.end);
    int length = image.tellg();
    image.seekg(0,image.beg);
    unsigned char * buffer = new unsigned char [length];

    image.read((char*)buffer,length);
    
    unsigned char *key = (unsigned char * ) "01234567890123456789012345678901";
    unsigned char *iv = (unsigned char * ) "0123456789012345";

    
    //unsigned char *plaintext = (unsigned char *) "hello this is going to be encrypted";

    //unsigned char ciphertext[500000];

    unsigned char decryptedtext[500000];

    int decryptedtext_len, ciphertext_len;

    //ciphertext_len = encrypt(buffer,length, key, iv, ciphertext);

    printf("ciphertext len: %d\n", length);

   // BIO_dump_fp(stdout,(const char *)ciphertext,ciphertext_len);

    decryptedtext_len = decrypt(buffer,length, key, iv, decryptedtext);

    //decryptedtext[decryptedtext_len] = '\0';

    //printf("Decrpted text\n");

    //printf("%s\n",decryptedtext);

    std::ofstream image_final("dec_new.png", std::ios::binary | std::ios::ate);
    image_final.write((char *)decryptedtext,decryptedtext_len);
    

    return 0;
}


