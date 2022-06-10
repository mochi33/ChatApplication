#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <arpa/inet.h>


pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

struct clientdata {
  int sock;
  struct sockaddr_in saddr;
};

char *iplist[20] = {
  "127.0.0.1",
};

struct memberData {
  int sock[5];
  char name[5][20];
  int n;
} memberdata = {
  .sock = {0},
  .name = {""},
  .n = 0,
};

int sendMessage (char buf[], int n, int sendernumber) {
  printf("%sを送信します。nは%dです\n", buf, memberdata.n);
  for (int i = 0; i < memberdata.n; i++) {
    printf("%sさんへ。\n",memberdata.name[i]);
    write(memberdata.sock[i], memberdata.name[sendernumber-1], 20);
    write(memberdata.sock[i], buf, n);
  }
  return 0;
}

void *threadfunc(void *data)
{
  int sock;
  struct clientdata *cdata = data;
  char nbuf[20];
  char buf[1024];
  int number;
  int n;
  int r;

  if (data == NULL) {
    return (void *)-1;
  }

  sock = cdata->sock;
  
  n = read(sock, nbuf, sizeof(nbuf));
  if (n < 0) {
      perror("readname");
      goto err;
    }
  
  r = pthread_mutex_lock(&m);
  if (r != 0) {
    perror("mutex_lock");
  }
  if (memberdata.n < 5) {
    memberdata.n++;
    strcpy(memberdata.name[memberdata.n-1], nbuf);
    memberdata.sock[memberdata.n-1] = sock;
    number = memberdata.n;
  } else {
    goto err;
  }
  r = pthread_mutex_unlock(&m);
  if (r != 0) {
    perror("mutex_unlock");
  }

  sendMessage("入室しました。", 22, number);

  while (1) {
    n = read(sock, buf, sizeof(buf));
    if (n < 0) {
      perror("read");
      goto err;
    } else if (n == 0){
      sendMessage("退出しました。", 22, number);
      r = pthread_mutex_lock(&m);
      if (r != 0) {
        perror("mutex_lock");
      }
      memset(memberdata.name[number-1], 0, 20);
      memberdata.sock[number-1] = 0;
      memberdata.n--;
      r = pthread_mutex_unlock(&m);
      if (r != 0) {
        perror("mutex_unlock");
      }
      break;
    }
    write(fileno(stdout), buf, n);
    sendMessage(buf, n, number);
  }

  if (close(sock) != 0) {
    perror("close");
    goto err;
  }
  free(data);

  return NULL;

err:
  free(data);
  return (void *)-1;
}

int main()
{
  int sock0;
  struct sockaddr_in addr;
  socklen_t len;
  pthread_t th;
  struct clientdata *cdata;

  sock0 = socket(AF_INET, SOCK_STREAM, 0);

  addr.sin_family = AF_INET;
  addr.sin_port = htons(12345);
  addr.sin_addr.s_addr = INADDR_ANY;

  bind(sock0, (struct sockaddr *)&addr, sizeof(addr));

  listen(sock0, 5);


  for (;;) {
    cdata = malloc(sizeof(struct clientdata));
    if (cdata == NULL) {
      perror("malloc");
      return 1;
    }

    len = sizeof(cdata->saddr);
    cdata->sock = accept(sock0, (struct sockaddr *)&cdata->saddr, &len);
    write(fileno(stdout), inet_ntoa(cdata->saddr.sin_addr), 10);
    
      for(int n = 0; n < 10; n++) {
        if (strcmp(inet_ntoa(cdata->saddr.sin_addr), iplist[n]) == 0) {
        if (pthread_create(&th, NULL, threadfunc, cdata) != 0) {
          perror("pthread_create");
          return 1;
        }

        if (pthread_detach(th) != 0) {
          perror("pthread_detach");
          return 1;
        }
        break;
      }
    }
    
  }

  if (close(sock0) != 0) {
    perror("close");
    return 1;
  }

  return 0;
}
