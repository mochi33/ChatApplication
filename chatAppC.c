#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

void *receiver (void *sock) {
    int *sock0;
    int n;
    char nbuf[20];
    char buf[1024];

    sock0 = sock;

    while (1) {
        n = read(*sock0, nbuf, sizeof(nbuf));
        printf("%s : ", nbuf);
        if (n < 0) {
            perror("read");
            goto err;
        } else if (n == 0) {
            break;
        }
        n = read(*sock0, buf, sizeof(buf));
        printf("%s\n",buf);
        printf("--------------------------\n");
    }

    return NULL;

    err:
        free(sock);
        return (void *)-1;
}

int main(int argc, char *argv[])
{
    //入力の1つ目がサーバー側のIP,2つ目がポート番号(12345固定)
    struct sockaddr_in server;
    struct addrinfo hints, *res;
    struct in_addr addr;
    pthread_t th;
    int sock;
    char name[20];
    char buf[1024];
    char ip[16];
    int n;
    int err;
    

    if (argc != 3) {
        fprintf(stderr, "Usage : %s filename\n", argv[0]);
        return 1;
        }

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
    if ((err = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
        printf("error %d\n", err);
        return 1;
    }

    addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;
    inet_ntop(AF_INET, &addr, ip, sizeof(ip));
    printf("ip address : %s\n", ip);
    
    freeaddrinfo(res);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, ip, &server.sin_addr.s_addr);
    connect(sock, (struct sockaddr *)&server, sizeof(server));
    while (1) {
        memset(name, 0, sizeof(name));
        printf("名前を入力してください(15文字以内)\n");
        scanf("%s", name);
        if (strlen(name) < 15) {
            n = write(sock, name, sizeof(name));
            if (n < 0){
                perror("writename");
                return 0;
            }
            break;
        }
    }

    if (pthread_create(&th, NULL, receiver, &sock) != 0) {
      perror("pthread_create");
      return 1;
    }

    if (pthread_detach(th) != 0) {
      perror("pthread_detach");
      return 1;
    }

    while (1) {
        memset(buf, 0, sizeof(buf));
        scanf("%s", buf);
        n = write(sock, buf, sizeof(buf));
        if (n < 0) {
            perror("write");
        }
    }

    close(sock);
    return 0;
}