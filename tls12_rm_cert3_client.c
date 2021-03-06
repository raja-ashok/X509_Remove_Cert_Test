#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include "openssl/crypto.h"
#include "openssl/ssl.h"

#define CADIR1 "./certs/ECC_Prime256_Certs2/cacert"
#define CADIR2 "./certs/ECC_Prime256_Certs/cacert"
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 7788
#define MAX_CONNECT_TRY 20
#define CONNECT_SLEEP_TIME 100 /* ms */

int do_tcp_connection(const char *server_ip, uint16_t port)
{
    struct sockaddr_in serv_addr;
    int fd;
    int ret;
    int i = 0;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        printf("Socket creation failed\n");
        return -1;
    }
    printf("Client fd=%d created\n", fd);

    serv_addr.sin_family = AF_INET;
    if (inet_aton(server_ip, &serv_addr.sin_addr) == 0) {
        printf("inet_aton failed\n");
        goto err_handler;
    }
    serv_addr.sin_port = htons(port);

    do {
        ret = connect(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        printf("sleeping...\n");
        usleep(CONNECT_SLEEP_TIME * 1000);
    } while ((i++ < MAX_CONNECT_TRY) && (ret != 0));
    if (ret) {
        printf("Connect failed, errno=%d\n", errno);
        goto err_handler;
    }
    /*ret = connect(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (ret) {
        printf("Connect failed, errno=%d\n", errno);
        goto err_handler;
    }*/
    
    printf("TLS connection succeeded, fd=%d\n", fd);
    return fd;
err_handler:
    close(fd);
    return -1;
}

int verify_cb(int ok, X509_STORE_CTX *ctx)
{
    //STACK_OK(X509) *chain;
    void *chain;
    printf("Verify Callback\n");
    if (ok) {
        chain = X509_STORE_CTX_get0_chain(ctx);
        if (chain) {
            printf("Cert Chain size=%d\n", sk_X509_num(chain));
        }
    } else {
        printf("Cert verify failed, error code %d\n", X509_STORE_CTX_get_error(ctx));
    }
    return ok;
}

SSL_CTX *create_context()
{
    SSL_CTX *ctx;

    ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        printf("SSL ctx new failed\n");
        return NULL;
    }

    printf("SSL context created\n");

    if (SSL_CTX_load_verify_locations(ctx, NULL, CADIR1) != 1) {
        printf("Load CA cert from dir %s failed\n", CADIR1);
        goto err_handler;
    }

    printf("Loaded cert from dir %s on context\n", CADIR1);

    if (SSL_CTX_load_verify_locations(ctx, NULL, CADIR2) != 1) {
        printf("Load CA cert from dir %s failed\n", CADIR2);
        goto err_handler;
    }

    printf("Loaded cert from dir %s on context\n", CADIR2);
    printf("Num of cert in store is %d\n",
            sk_X509_OBJECT_num(X509_STORE_get0_objects(SSL_CTX_get_cert_store(ctx))));

    /* Set verify callback */
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_cb);
    SSL_CTX_set_verify_depth(ctx, 5);
    SSL_CTX_set_options(ctx, SSL_OP_NO_TLSv1_3);

    printf("SSL context configurations completed\n");

    return ctx;
err_handler:
    SSL_CTX_free(ctx);
    return NULL;
}

SSL *create_ssl_object(SSL_CTX *ctx)
{
    SSL *ssl;
    int fd;

    fd = do_tcp_connection(SERVER_IP, SERVER_PORT);
    if (fd < 0) {
        printf("TCP connection establishment failed\n");
        return NULL;
    }

    ssl = SSL_new(ctx);
    if (!ssl) {
        printf("SSL object creation failed\n");
        return NULL; 
    }

    SSL_set_fd(ssl, fd);

    printf("SSL object creation finished\n");

    return ssl;
}

int tls12_client()
{
    SSL_CTX *ctx;
    SSL *ssl = NULL;
    int fd;
    int ret;

    ctx = create_context();
    if (!ctx) {
        return -1;
    }

    ssl = create_ssl_object(ctx);
    if (!ssl) {
        goto err_handler;
    }

    fd = SSL_get_fd(ssl);


    ret = SSL_connect(ssl); 
    if (ret != 1) {
        printf("Num of cert in store is %d\n",
            sk_X509_OBJECT_num(X509_STORE_get0_objects(SSL_CTX_get_cert_store(ctx))));
        printf("SSL connect failed%d\n", ret);
        goto err_handler;
    }

    printf("SSL connect succeeded\n");
    printf("Num of cert in store is %d\n",
            sk_X509_OBJECT_num(X509_STORE_get0_objects(SSL_CTX_get_cert_store(ctx))));
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(fd);

    return 0;
err_handler:
    if (ssl) {
        SSL_free(ssl);
    }
    SSL_CTX_free(ctx);
    close(fd);
    return -1;
}

int main()
{
    int ret = 0;
    printf("OpenSSL version: %s, %s\n", OpenSSL_version(OPENSSL_VERSION), OpenSSL_version(OPENSSL_BUILT_ON));
    if (tls12_client() != 0) {
        printf("TLS12 client connection failed\n");
        ret = -1;
    }
    return ret;
}
