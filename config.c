#define _POSIX_C_SOURCE 199309L
#define _XOPEN_SOURCE

#include "config.h"
#define BUF_SIZE_C 1000
extern char request[];

enum
{
  GET = 20,
  POST,
  HEAD
};

bool timeExpireFlag = false;
int piped[2];

int method = GET,
    time = 1,
    concurrent = 1;

char port[20] = "";
char host[100] = "";
char buf[BUF_SIZE_C + 1] = "";

struct option argvesOptions[] = {
    {"concurrent", required_argument, NULL, 'c'},
    {"time", required_argument, NULL, 't'},
    {"get", no_argument, &method, GET},
    {"post", no_argument, &method, POST},
    {"head", no_argument, &method, HEAD},
    {"help", no_argument, NULL, 'h'}};

void sig_handler(int sig)
{
  timeExpireFlag = true;
}

void usage()
{
  fprintf(stderr, "webBench [option]... URL\n"
                  "-c|--concurrent <n>     Run <n> HTTP clients at once. Default one.\n"
                  "-t|--time <n>           Run <n> seconds, deFault is 30 seconds\n"
                  "--get                   use get Method\n"
                  "--head                  use head method\n"
                  "--post                  use post method\n"
                  "--help|-h               help\n");
}

int get_long(char *str)
{
  int res = atoi(str);
  if (res <= 0)
  {
    res = 1;
  }
  return res;
}

int buildRequest(char *dest, char *src)
{
  if (strstr(src, "http://") == NULL)
  {
    fprintf(stderr, "wrong url \n");
    return -1;
  }
  char tmp[10];
  memset(tmp, '\0', 10);
  switch (method)
  {
  default:
  case GET:
    strcpy(tmp, "GET");
    break;
  case POST:
    strcpy(tmp, "POST");
    break;
  case HEAD:
    strcpy(tmp, "HEAD");
    break;
  }
  strcat(dest, tmp);
  strcat(dest, " ");
  char *location = strchr(src + 7, '/');
  if (!location)
  {
    fprintf(stderr, "no location\n");
    return -1;
  }
  strcat(dest, location);
  strcat(dest, " HTTP/1.1\r\n");
  strcat(dest, "Accept: */*\r\n");
  char *portLoc = strchr(src + 7, ':');
  if (!portLoc)
  {
    strcpy(port, "80");
    strcat(dest, "Host: ");
    size_t n = -7 + location - src;
    strncpy(host, src + 7, n);
    strncat(dest, src + 7, n);
    strcat(dest, "\r\n");
  }
  else
  {
    strncpy(host, src + 7, portLoc - src - 7);
    strncpy(port, portLoc, location - portLoc);
  }
  strcat(dest, "\r\n");
  return 1;
}

void bullet()
{
  int fail = 0,
      success = 0;
  if (pipe(piped) == -1)
  {
    fprintf(stderr, "error in pipe\n");
    exit(1);
  }
  pid_t pid;
  for (int i = 0; i < concurrent; i++)
  {
    pid = fork();
    if (pid < 0)
    {
      fprintf(stderr, "error in fork :%s", strerror(errno));
      exit(1);
    }
    else if (pid == 0)
    {
      sleep(2);
      break;
    }
  }
  if (pid == 0)
  {
    //child
    close(piped[0]);
    childWork(&success, &fail);
    printf("%d:%d\n", success, fail);
    FILE* fp = fdopen(piped[1], "w");
    // printf("%d, %p\n", piped[1],fp);
    if(fp == NULL) {
      fprintf(stderr, "Err in fdopen child: %s\n",strerror(errno));
      exit(1);
    }
    setbuf(fp, NULL);
    fprintf(fp, "%d:%d\n", success, fail); 
    fclose(fp);

    return ;
  }
  else
  {
    //parent
    close(piped[1]);
    int i = 0,j = 0;
    FILE* fp = fdopen(piped[0], "r");
    if(fp == NULL) {
      fprintf(stderr, "Err in fdopen pare: %s\n",strerror(errno));
      exit(1);
    }
    setbuf(fp, NULL);

    // while (wait(NULL)> 0) {
      while(fscanf(fp,  "%d:%d", &i, &j) != EOF)
      {
        // printf("total: %d:%d\n", i, j);
        success += i;
        fail += j;
        // if(--concurrent == 0) {
        //   break;
        // }
        // printf("%d\n", concurrent);
      }
    // }
    printf("total: %d:%d\n", success, fail);
  }
}

void childWork(int *suc_ptr, int *fail_ptr)
{
  struct sigaction act;
  memset(&act, 0, sizeof(act));
  sigemptyset(&act.sa_mask);
  act.sa_handler = sig_handler;
  if (sigaction(SIGALRM, &act, NULL) == -1)
  {
    fprintf(stderr, "error in sigaction : %s\n", strerror(errno));
    exit(1);
  }
  alarm(time);
restart:
  while (1)
  {
    if (timeExpireFlag)
    {
      if (*fail_ptr > 0)
      {
        (*fail_ptr)--;
      }
      break;
    }
    int fd = Socket();
    if (fd < 0)
    {
      fprintf(stderr, "error in socket"
                      "in childwork: %s\n",
              strerror(errno));
      (*fail_ptr)++;
      continue;
    }
    int len = strlen(request),
        readLen;
    readLen = write(fd, request, len);
    if (readLen != len)
    {
      fprintf(stderr, "error Write:%s\n", strerror(errno));
      (*fail_ptr)++;
      close(fd);
      continue;
    }
    if (shutdown(fd, SHUT_WR) < 0)
    {
      fprintf(stderr, "shutdown: %s\n", strerror(errno));
      (*fail_ptr)++;
      close(fd);
      continue;
    }

    while (readLen > 0)
    {
      if (timeExpireFlag)
      {
        (*fail_ptr)++;
        close(fd);
        goto restart;
      }
      readLen = read(fd, buf, BUF_SIZE_C);
      // buf[BUF_SIZE_C] = '\0';
      // fprintf(stderr, "read: %s\n", buf);
    }
    if (readLen == -1)
    {
      fprintf(stderr, "error Write:%s\n", strerror(errno));
      (*fail_ptr)++;
      close(fd);
      continue;
    }
    close(fd);
    (*suc_ptr)++;
  }
}

int Socket()
{
  struct addrinfo hint, *res;
  memset(&hint, 0, sizeof(hint));
  hint.ai_socktype = SOCK_STREAM;
  hint.ai_family = AF_INET;
  hint.ai_protocol = 0;
  hint.ai_flags = NULL;
  int error;
  if ((error = getaddrinfo(host, port, &hint, &res)) < 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
    return -1;
  }
  int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (fd < 0)
  {
    fprintf(stderr, "socket error \n");
    return -1;
  }

  if (connect(fd, res->ai_addr, sizeof(struct sockaddr_in)) < 0)
  {
    fprintf(stderr, "error in connect: %s\n", strerror(errno));
    close(fd);
    return -1;
  }
  freeaddrinfo(res);
  return fd;
}