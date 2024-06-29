#include "idobata.h"

#define S_BUFSIZE 100   /* 送信用バッファサイズ */
#define R_BUFSIZE 100   /* 受信用バッファサイズ */
#define NAMELENGTH 100 /* ログイン名の長さ */

void client_main(struct sockaddr_in server_adrs, char *user_name, in_port_t port)
{
  int sock, from_len;
  int broadcast_sw = 1;
  char helo[] = "HELO";
  char here[] = "HERE";
  char name[] = "JOIN ";
  char s_buf[S_BUFSIZE], r_buf[BUFSIZE];
  int strsize;
  fd_set mask, readfds;
  struct timeval timeout;
  WINDOW *win_main, *win_sub;

  /* ソケットをSTREAMモードで作成する */
  /* 画面の初期化 */
  init_window(&win_main, &win_sub);

  /* サーバに接続する */
  sock = init_tcpclient_ip(server_adrs, port);
  show_message_main(&win_main, "Connected.\n");
  /* ログイン名を送信する */
  strsize = strlen(user_name) + strlen(name);
  strcat(name, user_name);
  name[strsize] = '\0';
  Send(sock, name, strlen(name), 0);

  /* ビットマスクの準備 */
  FD_ZERO(&mask);
  FD_SET(0, &mask);
  FD_SET(sock, &mask); //sockはint型であるので、これでいい

  for(;;){
    /* 受信データの有無をチェック */
    readfds = mask;
    select( sock+1, &readfds, NULL, NULL, NULL );

    if( FD_ISSET(0, &readfds) ){
      /* キーボードから文字列を入力する */
      strsize = input_message_sub(&win_sub, s_buf, S_BUFSIZE);
      if(strsize == 0) continue; /* 文字列が空のときは送信しない */
      if(strcmp(s_buf, "QUIT") == 0){
        /* サーバにQUITメッセージを送信してから終了する */
        Send(sock, "QUIT", 4, 0);
        break;
      }
      char msg[S_BUFSIZE] = "POST ";
      strsize = strlen(s_buf) + strlen(msg);
      strcat(msg, s_buf);
      msg[strsize] = '\0';
      strsize = strlen(msg);
      Send(sock, msg, strsize, 0);
      /*ここで送信した文字列も表示*/
      wattrset(win_main,COLOR_PAIR(2));	/* 文字色を変更 */
      wprintw(win_main,"\t\t[%s] %s\n",user_name, s_buf);
      wrefresh(win_main);
    }

    if( FD_ISSET(sock, &readfds) ){
      /* サーバから文字列を受信する */
      strsize = Recv(sock, r_buf, R_BUFSIZE-1, 0);
      r_buf[strsize] = '\0';
      char *header = strtok(r_buf, " ");
      if(strcmp(header, "MESG") != 0){
        continue;
      }
      //ここを抜粋して答える
      //FINパケットを受け取った時，文字長は0になる
      if(strsize == 0){
        /* サーバが切断したとき */
        printf("Connection closed.\n");
        break;
      }
      header = strtok(NULL, "\0");
      show_message_main(&win_main, header);
    }

  }

  close(sock);
  endwin();
  exit(EXIT_SUCCESS);
}