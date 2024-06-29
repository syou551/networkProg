#include "idobata.h"

#define NAMELENGTH 20 /* ログイン名の長さ制限 */
#define BUFLEN 500    /* 通信バッファサイズ */
#define SUBWIN_LINES 2 /* サブウィンドウの行数 */

/* 各クライアントのユーザ情報を格納する構造体の定義 */
typedef struct{
  int  sock;
  char name[NAMELENGTH];
} client_info;

/* プライベート変数 */
static int N_client;         /* クライアントの数 */
static client_info *Client;  /* クライアントの情報 */
static int Max_sd;               /* ディスクリプタ最大値 */
static char Buf[BUFLEN];     /* 通信用バッファ */
static char *Message; /* メッセージバッファ */
static int sender;

/* プライベート関数 */
static int client_login(int sock_listen);
static void send_message();
static int receive_message();
static void delete_member(int client_id);

char *chop_nl(char *s);

/* ウィンドウの初期化 */
void init_window(WINDOW **win_main, WINDOW **win_sub)
{
  initscr();
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  setlocale(LC_ALL, "");
  // 色の準備
  *win_main = newwin(LINES - SUBWIN_LINES, COLS, 0, 0); /* Windowを作る */
  *win_sub  = newwin(SUBWIN_LINES, COLS, LINES-SUBWIN_LINES, 0);
  scrollok(*win_main, TRUE); /* スクロールを許可する */
  scrollok(*win_sub, TRUE);
  wmove(*win_main, 0,20);  /* カーソルを動かす */
  wprintw(*win_main, "TCP Chat Client Program\n");
  wrefresh(*win_main);  /* 画面を更新 */
  wrefresh(*win_sub);
}

/* メインウィンドウにメッセージを表示 */
void show_message_main(WINDOW **win_main, char *message)
{
  wattrset(*win_main,COLOR_PAIR(1));	/* 文字色を変更 */
  wprintw(*win_main, "%s", message);
  wrefresh(*win_main);
}

/* サブウィンドウから入力を受け取る */
int input_message_sub(WINDOW **win_sub, char *buf, int S_BUFSIZE)
{
  wgetnstr(*win_sub, buf, S_BUFSIZE);
  werase(*win_sub);
  wrefresh(*win_sub);
  return strlen(buf);
}

char *chop_nl(char *s)
{
  int len;
  len = strlen(s);

  if( len>0 && (s[len-1]=='\n' || s[len-1]=='\r') ){
    s[len-1] = '\0';
    if( len>1 && s[len-2]=='\r'){
      s[len-2] = '\0';
    }
  }

  return s;
}