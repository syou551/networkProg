#include "idobata.h"
//#define DEBUG

typedef struct _login_arg {
  int sock_listen;
  struct sockaddr_in from_adrs;
  pthread_t tid;
}login_arg;

void server_main(char *user_name, in_port_t port)
{
  int udp_sock, tcp_sock_listen, from_len;
  int broadcast_sw = 1;
  struct sockaddr_in from_adrs;
  char helo[] = "HELO";
  char here[] = "HERE";
  char s_buf[1024], r_buf[BUFSIZE];
  int strsize;
  fd_set mask, readfds;
  struct timeval timeout;
  login_arg * arg;
  pthread_t tid;

  /* UDPソケットの初期化 */
  udp_sock = init_udpserver(port);
  tcp_sock_listen = init_tcpserver(port,5);
  from_len = sizeof(from_adrs);
  /* 監視するsockの最大値 */
  int maxfd = udp_sock;
  FD_ZERO(&mask);
  FD_SET(udp_sock, &mask);
  /* スレッド関数の引数を用意する */
  if( (arg = (login_arg *)malloc(sizeof(login_arg))) == NULL ){
    exit_errmesg("malloc()");
  }
  arg->sock_listen = tcp_sock_listen;

  /*チャットスレッドの作成*/
  if( pthread_create(&tid, NULL, chat_loop,(void *)NULL) != 0 ){
    exit_errmesg("pthread_create()");
  }
  arg->tid = tid;
  while(true){
    /* 受信データの有無をチェック */
    readfds = mask;
    if(select( maxfd+1, &readfds, NULL, NULL, NULL )==-1){
      printf("select()");
      continue;
    };

    if( FD_ISSET(udp_sock, &readfds) ){
      strsize = Recvfrom(udp_sock, r_buf, BUFSIZE-1, 0,(struct sockaddr *)&from_adrs,&from_len);
      r_buf[strsize] = '\0';
      if( strncmp(r_buf, helo, 4) == 0 ){
        sendto(udp_sock, here, strlen(here), 0, (struct sockaddr *)&from_adrs,sizeof(from_adrs));
        arg->from_adrs = from_adrs;
        /* スレッドを生成する */
        if( pthread_create(&tid, NULL, client_login, (void *)arg) != 0 ){
          exit_errmesg("pthread_create()");
        }
      }
    }
  }

}