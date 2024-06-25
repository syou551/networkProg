#include "idobata.h"

void client_main(struct sockaddr_in server_adrs, char *user_name, in_port_t port)
{
  struct sockaddr_in from_adrs;
  int sock, from_len;
  int broadcast_sw = 1;
  char helo[] = "HELO";
  char here[] = "HERE";
  char s_buf[1024], r_buf[BUFSIZE];
  int strsize;
  fd_set mask, readfds;
  struct timeval timeout;

  /* ソケットをSTREAMモードで作成する */
  

  
}