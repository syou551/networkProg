#include "idobata.h"
//#define DEBUG

typedef struct _login_arg {
  int sock_listen;
  struct sockaddr_in from_adrs;
  pthread_t tid;
}login_arg;

typedef struct _chat_arg {
  WINDOW *win_main;
  WINDOW *win_sub;
}chat_arg;

void server_main(char *user_name, in_port_t port)
{
  int udp_sock, tcp_sock_listen, from_len;
  int broadcast_sw = 1;
  struct sockaddr_in from_adrs;
  char helo[] = "HELO";
  char here[] = "HERE";
  char s_buf[BUFSIZE], r_buf[BUFSIZE];
  int strsize;
  fd_set mask, readfds;
  struct timeval timeout;
  login_arg * arg;
  chat_arg * cht_arg;
  pthread_t tid;
  WINDOW *win_main, *win_sub;

  /* 画面の初期化 */
  init_window(&win_main, &win_sub);

  /* UDPソケットの初期化 */
  udp_sock = init_udpserver(port);
  tcp_sock_listen = init_tcpserver(port,5);
  from_len = sizeof(from_adrs);
  /* 監視するsockの最大値 */
  int maxfd = udp_sock;
  FD_ZERO(&mask);
  FD_SET(0, &mask);
  FD_SET(udp_sock, &mask);
  /* スレッド関数の引数を用意する */
  if( (arg = (login_arg *)malloc(sizeof(login_arg))) == NULL ){
    exit_errmesg("malloc()");
  }
  arg->sock_listen = tcp_sock_listen;
    if( (cht_arg = (chat_arg *)malloc(sizeof(chat_arg))) == NULL ){
    exit_errmesg("malloc()");
  }
  cht_arg->win_main = win_main;
  cht_arg->win_sub = win_sub;

  /*チャットスレッドの作成*/
  if( pthread_create(&tid, NULL, chat_loop,(void *)cht_arg) != 0 ){
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
    }else if( FD_ISSET(0, &readfds) ){
      /* キーボードから文字列を入力する */
      strsize = input_message_sub(&win_sub, s_buf, BUFSIZE);
      if(strsize == 0) continue; /* 文字列が空のときは送信しない */
      else if(strcmp(s_buf, "QUIT") == 0){
        /* 終了する */
        close(udp_sock);
        close(tcp_sock_listen);
        break;
      }
      char msg[BUFSIZE];
      strsize = snprintf(msg, BUFSIZE, "MESG [%s] %s", user_name, s_buf);
      msg[strsize] = '\0';
      send_message_from_server(msg);
      /*ここで送信した文字列も表示*/
      wattrset(win_main,COLOR_PAIR(2));	/* 文字色を変更 */
      wprintw(win_main,"\t\t[%s] %s\n",user_name, s_buf);
      wrefresh(win_main);
    }
  }

}