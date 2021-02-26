#ifndef SSL_H
#define SSL_H

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include"global.h"

// #define CERTF "SSL/server/server-cert.pem"
// #define KEYF "SSL/server/server-key.pem"
// #define CAFILE "SSL/ca/ca-cert.pem"

#define CERTF "SSL/server_domain/server-cert.pem"
#define KEYF "SSL/server_domain/server-key.pem"
#define CAFILE "SSL/ca/ca-cert.pem"

SSL_CTX* InitSSL();
SSL* AttachSSLWithSocket(int sockId, SSL_CTX* ctx);
Bool VerifyX509(SSL* ssl);
void FreeSSL(SSL *ssl, SSL_CTX *ctx);

#endif