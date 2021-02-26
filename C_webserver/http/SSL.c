#include "SSL.h"

SSL_CTX* InitSSL()
{
    SSL_library_init();                                 // 初始化SSL库
    OpenSSL_add_all_algorithms();                       //载入所有SSL算法
    SSL_load_error_strings();                           //载入所有错误信息

    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
    if(ctx==NULL)
    {
#ifdef OUTPUT_ERROR_INFOR
        printf("初始化CTX失败\n");
#endif
        return NULL;
    }

    // 要求校验对方证书
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    // 加载CA的证书
    SSL_CTX_load_verify_locations(ctx, CAFILE, NULL); //设置CA

    // 加载自己的证书
    if (SSL_CTX_use_certificate_file(ctx, CERTF, SSL_FILETYPE_PEM)<=0)
    {
        ERR_print_errors_fp(stderr);
#ifdef OUTPUT_ERROR_INFOR
        printf("加载认证证书出错\n");
#endif
        return NULL;
    }

    // 加载自己的私钥
    if (SSL_CTX_use_PrivateKey_file(ctx, KEYF, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
#ifdef OUTPUT_ERROR_INFOR
        printf("加载私钥出错\n");
#endif
        return NULL;
    }

    // 验证私钥是否正确
    if (!SSL_CTX_check_private_key(ctx))
    {
#ifdef OUTPUT_ERROR_INFOR
        printf("公钥与私钥不匹配\n");
#endif
        return NULL;
    }

//     SSL_CTX_set_client_CA_list(ctx, SSL_load_client_CA_file(CAFILE));
//    // SSL_CTX_set_cipher_list(ctx, "RC4-MD5");

   SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);
    return ctx;
}

SSL* AttachSSLWithSocket(int sockId, SSL_CTX* ctx)
{
    if(ctx==NULL)
    {
#ifdef OUTPUT_ERROR_INFOR
        printf("计算SSL时，ctx=NULL \n");
#endif
        return NULL;
    }

    SSL* ssl = SSL_new(ctx);
    if (ssl==NULL)
    {
#ifdef OUTPUT_ERROR_INFOR
        printf("初始化SSL时发生错误\n");
#endif
        return NULL;
    }

    //绑定套接字与SSL
    if (SSL_set_fd(ssl, sockId)==0)
    {
#ifdef OUTPUT_ERROR_INFOR
        printf("关联SSL与套接字出错\n");
#endif
        return NULL;
    }

    for (int i = 0; i < 100;i++)
    {
        int k = SSL_accept(ssl);
        if (k<=0)
        {
            continue;
        }
        else
        {
            if(!VerifyX509(ssl))
            {
                SSL_free(ssl);
                return NULL;
            }

            return ssl;
        }
    }

#ifdef OUTPUT_ERROR_INFOR
    printf("SSL连接出错, 信息: %s \n", ERR_reason_error_string(ERR_get_error()));
#endif
    return NULL;
}

Bool VerifyX509(SSL* ssl)
{

    X509 *client_cert = SSL_get_peer_certificate(ssl);

#ifdef OUTPUT_DEBUG_INFO
    printf("\n用户证书如下:\n");
#endif

    if (client_cert != NULL)
    {
        char *str = X509_NAME_oneline(X509_get_subject_name(client_cert), 0, 0);
        if (!str)
        {
#ifdef OUTPUT_ERROR_INFOR
            printf("subject\n");
#endif
            return false;
        }

#ifdef OUTPUT_DEBUG_INFO
        printf("subject: %s\n", str);
#endif
        OPENSSL_free(str);

        str = X509_NAME_oneline(X509_get_issuer_name(client_cert), 0, 0);
        if (!str)
        {
#ifdef OUTPUT_ERROR_INFOR
            printf("issuer存在错误\n");
#endif
            return false;
        }

#ifdef OUTPUT_DEBUG_INFO
        printf("issuer: %s\n\n", str);
#endif
        X509_free(client_cert);
        OPENSSL_free(str);
        return true;
    }
    else
    {
#ifdef OUTPUT_ERROR_INFOR
        printf("用户证书不存在\n");
#endif
        return false;
    }
}

void FreeSSL(SSL* ssl, SSL_CTX* ctx)
{
    if(!ssl)
    {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        ssl = NULL;
    }

    if(!ctx)
    {
        SSL_CTX_free(ctx);
        ctx = NULL;
    }
}

// SSL_write(ssl, "Server is connect to you!\n", strlen("Server is connect to you!\n"));
// SSL_read(ssl, buf, sizeof(buf));