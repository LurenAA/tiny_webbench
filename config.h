#ifndef __Asdasd_ss
#define __Asdasd_ss

#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/wait.h>

void usage();
int get_long(char *str);
int buildRequest(char *dest, char *src);
void bullet();
int Socket();
void childWork(int *success, int *fail);

#endif