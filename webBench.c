#include "config.h"

extern 
struct option argvesOptions[];

extern 
int time, 
  concurrent;

#define MAX_HTTP_SIZE 500
char request[MAX_HTTP_SIZE] = "";

int main(int argc, char* argv[]) 
{
  if(argc == 1) {
    usage();
    exit(EXIT_FAILURE);
  }
  int *longIndex,
    opt;
  while((opt = getopt_long(argc, argv, "t:c:h"
    ,argvesOptions, longIndex)) != -1)
  {
    switch (opt)
    {
    default: 
    case '?':
    case 'h':
      usage();
      exit(EXIT_FAILURE);
      break;
    case 't':
      time = get_long(optarg);
      break;
    case 'c':
      concurrent = get_long(optarg);
      break;
    }
  }
  if(optind == argc) {
    fprintf(stderr, "missing url\n");
    exit(EXIT_FAILURE);
  }
  if(buildRequest(request, argv[optind]) == -1) {
    return EXIT_FAILURE;
  }
  
  //test network
  int fd = Socket();
  if(fd < 0) {
    fprintf(stderr, "error socket\n");
    exit(1);
  }

  bullet();
  return EXIT_SUCCESS;
}