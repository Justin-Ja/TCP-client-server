#ifndef _TCPHEADERCODE
#define _TCPHEADERCODE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <asm-generic/siginfo.h>
#include <time.h>

#define DEFAULT_BUFSIZE 4096

#define MAX_QUEUE_LEN 10

void checkArgv(int numArgs, char *givenArguments[]);

#endif