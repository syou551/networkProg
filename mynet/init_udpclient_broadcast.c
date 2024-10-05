#include "mynet.h"

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

void set_sockaddr_in_broadcast(struct sockaddr_in *server_adrs, 
		               in_port_t port_number )
{
  /* ブロードキャストアドレスの情報をsockaddr_in構造体に格納する */
  memset(server_adrs, 0, sizeof(struct sockaddr_in));
  server_adrs->sin_family = AF_INET;
  server_adrs->sin_port = htons(port_number);
  server_adrs->sin_addr.s_addr = htonl(INADDR_BROADCAST);
}
