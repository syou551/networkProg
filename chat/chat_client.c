#include "chat.h"
#include "../mynet/mynet.h"
#include <stdlib.h>
#include <sys/select.h>
#include <curses.h>
#include <ncurses.h>
#include <locale.h>

#define S_BUFSIZE 100   /* 送信用バッファサイズ */
#define R_BUFSIZE 100   /* 受信用バッファサイズ */
#define NAMELENGTH 100 /* ログイン名の長さ */

void chat_client(char* servername, int port_number)
{
  int sock;
  char s_buf[S_BUFSIZE], r_buf[R_BUFSIZE];
  static char name[NAMELENGTH];
  int strsize;
  fd_set mask, readfds;
  WINDOW *win_main, *win_sub;

  /* 画面の初期化 */
  init_window(&win_main, &win_sub);
  
  setlocale(LC_ALL, "");

  /* サーバに接続する */
  sock =  init_tcpclient(servername, port_number);
  wprintw(win_main,"Connected.\n");
  wrefresh(win_main);
  /* ログインプロンプトの受信 */
  strsize = Recv(sock, r_buf, R_BUFSIZE-1, 0);
  r_buf[strsize] = '\0';
  wprintw(win_main,"%s",r_buf);
  wrefresh(win_main);
  /* ログイン名の入力 */
  wgetnstr(win_sub, name, NAMELENGTH);
  name[strlen(name)] = '\0';
  Send(sock, name, strlen(name), 0);
  werase(win_sub);
  wrefresh(win_sub);
  wprintw(win_main," %s\n",name);
  wrefresh(win_main);

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
      wgetnstr(win_sub, s_buf, S_BUFSIZE);
      werase(win_sub);
      if(strlen(s_buf) == 0) continue; /* 文字列が空のときは送信しない */
      strcat(s_buf, "\n");
      strsize = strlen(s_buf);
      Send(sock, s_buf, strsize, 0);
      wrefresh(win_sub);
      /*ここで送信した文字列も表示*/
      wattrset(win_main,COLOR_PAIR(2));	/* 文字色を変更 */
      wprintw(win_main,"\t\t%s >From %s\n",chop_nl(s_buf), name);
      wrefresh(win_main);
    }

    if( FD_ISSET(sock, &readfds) ){
      /* サーバから文字列を受信する */
      strsize = Recv(sock, r_buf, R_BUFSIZE-1, 0);
      //ここを抜粋して答える
      //FINパケットを受け取った時，文字長は0になる
      if(strsize == 0){
        /* サーバが切断したとき */
        printf("Connection closed.\n");
        break;
      }
      r_buf[strsize] = '\0';
      wattrset(win_main,COLOR_PAIR(1));	/* 文字色を変更 */
      wprintw(win_main,"%s",r_buf);
      wrefresh(win_main);
    }

  }

}