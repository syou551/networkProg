#include "mynet.h"

int init_tcpclient_ip(struct sockaddr_in server_adrs, in_port_t serverport)
{
  struct hostent *server_host;
  int sock;

  /* ソケットをSTREAMモードで作成する */
  if((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1){
    exit_errmesg("socket()");
  }

  /* ソケットにサーバの情報を対応づけてサーバに接続する */
  if(connect( sock, (struct sockaddr *)&server_adrs, sizeof(server_adrs) )== -1){
    exit_errmesg("connect()");
  }

  return(sock);
}
