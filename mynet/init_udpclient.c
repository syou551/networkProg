#include "mynet.h"

int init_udpclient()
{
  int sock;

  /* ソケットをDGRAMモードで作成する */
  if((sock = socket(PF_INET, SOCK_DGRAM, 0)) == -1){
    exit_errmesg("socket()");
  }

  return(sock);
}

int init_udpclient_broadcast(int broadcast_sw)
{
  int sock;

  /* ソケットをDGRAMモードで作成する */
  sock = socket(PF_INET, SOCK_DGRAM, 0);

  /* ソケットをブロードキャスト可能にする */
  if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST,(void *)&broadcast_sw, sizeof(broadcast_sw)) == -1){
    exit_errmesg("setsockopt()");
  }

  return(sock);
}

void set_sockaddr_in(struct sockaddr_in *server_adrs, 
		     char *servername, in_port_t port_number )
{
  struct hostent *server_host;

  /* サーバ名をアドレス(hostent構造体)に変換する */
  if((server_host = gethostbyname( servername )) == NULL){
    exit_errmesg("gethostbyname()");
  }

  /* サーバの情報をsockaddr_in構造体に格納する */
  memset(server_adrs, 0, sizeof(struct sockaddr_in));
  server_adrs->sin_family = AF_INET;
  server_adrs->sin_port = htons(port_number);
  memcpy(&(server_adrs->sin_addr), server_host->h_addr_list[0], server_host->h_length);
}


void set_sockaddr_in_broadcast(struct sockaddr_in *server_adrs, 
		               in_port_t port_number )
{
  /* ブロードキャストアドレスの情報をsockaddr_in構造体に格納する */
  memset(server_adrs, 0, sizeof(struct sockaddr_in));
  server_adrs->sin_family = AF_INET;
  server_adrs->sin_port = htons(port_number);
  server_adrs->sin_addr.s_addr = htonl(INADDR_BROADCAST);
}
