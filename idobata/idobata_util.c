#include "idobata.h"

#define NAMELENGTH 20 /* ログイン名の長さ制限 */
#define BUFLEN 500    /* 通信バッファサイズ */
#define SUBWIN_LINES 2 /* サブウィンドウの行数 */
#define TIMEOUT_SEC 5 /* タイムアウト秒 */
//#define DEBUG

/* 各クライアントのユーザ情報を格納する構造体の定義 */
typedef struct _client_info{
  char name[NAME_LENGTH];
  int sock;
  struct _client_info *next;
}client_info;

/* メッセージ情報を格納する構造体の定義 */
typedef struct _message_info{
  int sender_sock;
  char message[BUFLEN];
  struct _message_info *next;
}message_info;

/* ログインスレッドの引数 */
struct login_arg{
  int sock_listen;
  struct sockaddr_in from_adrs;
  pthread_t tid;
};

/* チャットスレッドの引数 */
typedef struct _chat_arg {
  WINDOW *win_main;
  WINDOW *win_sub;
}chat_arg;

/* プライベート変数 */
static int N_client;         /* クライアントの数 */
static client_info *Client = NULL;  /* クライアントの情報 */
static client_info* last_client = NULL;
static int Max_sd;               /* ディスクリプタ最大値 */
static char Buf[BUFLEN];     /* 通信用バッファ */
static message_info *message = NULL;
static chat_arg* cht_arg;

/* プライベート関数 */
static void send_message();
static int receive_message();
static void delete_member(int client_id);
static void action_add_client(int signo);
static void set_action_client();
static void clr_action_client();
static void add_message_list(client_info *client, char *_mess, message_info **prev_message);

char *chop_nl(char *s);

/* ログインスレッド本体 */
void * client_login(void *arg)
{
  int sock_listen;
  struct sockaddr_in from_adrs;
  int sock_accepted;
  int strsize;
  struct sockaddr_in client_adrs;
  socklen_t client_adrs_len;
  char buf[BUFLEN];
  char join[] = "JOIN";
  client_info *client;
  struct sigaction action;
  struct login_arg *login_arg;

  client = malloc(sizeof(client_info));
  login_arg = (struct login_arg *)arg;
  sock_listen = login_arg->sock_listen;
  from_adrs = login_arg->from_adrs;

  pthread_detach(pthread_self()); /* スレッドの分離(終了を待たない) */
  /* シグナルハンドラを設定する */
  action.sa_handler = action_timeout;
  //割り込みを無効にする
  if(sigfillset(&action.sa_mask) == -1){
    exit_errmesg("sigfillset()");
  }
  action.sa_flags = 0;
  if(sigaction(SIGALRM, &action, NULL) == -1){
    exit_errmesg("sigaction()");
  }

  /* クライアントのログイン */
  /* タイムアウトの時間を設定 */
  alarm(TIMEOUT_SEC);
  /* クライアントの受付 */
  client_adrs_len = sizeof(client_adrs);
  sock_accepted = Accept(sock_listen, (struct sockaddr *)&client_adrs, &client_adrs_len);
  if(sock_accepted == -1){
    if(errno == EINTR){
      printf("Time out!\n");
      exit(EXIT_FAILURE);
    }else
    exit_errmesg("Accept()");
  }else if(client_adrs.sin_addr.s_addr != from_adrs.sin_addr.s_addr){
    close(sock_accepted);
    exit(EXIT_FAILURE);
  }
  alarm(0); /* alarmをリセット */
  client->sock = sock_accepted;

  /* クライアントの名前を受信 */
  strsize = Recv(sock_accepted, buf, NAME_LENGTH+5, 0);
  buf[strsize] = '\0';
  if(strncmp(buf, join, 4) != 0){
    exit(EXIT_FAILURE);
  }
  strcpy(client->name, buf+5);
  chop_nl(client->name);

  /* クライアントリストへの追加　*/
  if(Client == NULL){
    Client = client;
    client->next = NULL;
    last_client = Client;
  }else{
    last_client->next = client;
    last_client = client;
    last_client->next = NULL;
  }
#ifdef DEBUG
  /* クライアントの名前を表示 */
  printf("Client[%s] sock:[%d].\n", last_client->name,last_client->sock);
#endif
  pthread_kill(login_arg->tid, SIGUSR1);
#ifdef DEBUG
  printf("send signal.\n");
#endif
  return(NULL);
}

/* Acceptタイムアウト時に呼ばれるAction */
void action_timeout(int signo){
  return;
}

/* チャットスレッド本体 */
void * chat_loop(void *arg){
  cht_arg = (chat_arg *)arg;
#ifdef DEBUG
  printf("Chat loop start.\n");
#endif
  for(;;){
    /* メッセージの受け取り */
    if(receive_message()==-1) {
      continue;
    };
    /* メッセージを他ユーザーへ送信 */
    send_message();
  }
}

/* Client追加時に送られるSignalのAction */
static void action_add_client(int signo){
  //printf("New Client.\n");
  return;
}

/* クライアントの追加Event通知の設定 */
static void set_action_cleint(){
#ifdef DEBUG
  printf("Set Event.\n");
#endif
  struct sigaction action;
  action.sa_handler = action_add_client;
  if(sigfillset(&action.sa_mask) == -1){
    exit_errmesg("sigfillset()");
  }
  action.sa_flags = 0;
  if(sigaction(SIGUSR1, &action, NULL) == -1){
    exit_errmesg("sigaction()");
  }
}

/* クライアントの追加Event通知の解除 */
static void clr_action_client(){
#ifdef DEBUG
  printf("Clear Event.\n");
#endif
  struct sigaction action;
  action.sa_handler = SIG_IGN;
  if(sigfillset(&action.sa_mask) == -1){
    exit_errmesg("sigfillset()");
  }
  action.sa_flags = 0;
  if(sigaction(SIGUSR1, &action, NULL) == -1){
    exit_errmesg("sigaction()");
  }
}


/* クライアントからのメッセージの受信 */
static int receive_message()
{
  fd_set mask, readfds;
  int client_sock;
  int strsize;
  char buf[BUFLEN];

  /* ビットマスクの準備 */
  FD_ZERO(&mask);
  client_info *client = NULL;
  client = Client;
  int max_fd = 0;
  while(client != NULL){
    FD_SET(client->sock, &mask);
    max_fd = (max_fd > (client->sock))?max_fd:client->sock;
    client = client->next;
  }
  /* 受信データの有無をチェック */
  readfds = mask;
  /* クライアント追加のEvent通知 */
  set_action_cleint();
  if(select( max_fd+1, &readfds, NULL, NULL, NULL )==-1){
#ifdef DEBUG
  printf("select() intterupt.\n");
#endif
    /* クライアント追加Eventによる割り込み */
    if(errno == EINTR){
      /* 通知Event解除 */
      clr_action_client();
      return -1;
    }else{
      exit_errmesg("select()");
    }
  };
  /* 通知Event解除 */
  clr_action_client();

  client = Client;
  message_info *prev_message = NULL;
  while( client != NULL ){
#ifdef DEBUG
  printf("search sender...\n");
#endif
    if( FD_ISSET(client->sock, &readfds) ){
#ifdef DEBUG
  printf("find sender!\n");
#endif
      strsize = Recv(client->sock, buf, BUFLEN-1,0);
      if(strsize == 0){
        /*Clientがエラーで落ちてしまった場合の対処*/
        client_info *_delete = client;
        client = client->next;
        delete_member(_delete->sock);
        continue;
      }
      buf[strsize] = '\0';
#ifdef DEBUG
  printf("%s\n",buf);
#endif
      if(strsize == 4 && strncmp(buf, "QUIT", 4) == 0){
        client_info *_delete = client;
        client = client->next;
        delete_member(_delete->sock);
        continue;
      }
      if(strncmp(buf,"POST",4) != 0){
        printf("Invalid message format.\n");
        client = client->next;
        continue;
      }
      /*メッセージのリストへ登録*/
      add_message_list(client, chop_nl(buf+5), &prev_message);
      FD_CLR(client->sock, &mask);
#ifdef DEBUG
  printf("message put in list.\n");
#endif
    }
    client = client->next;
  }
  if(message != NULL)return 0;
  else return -1;
}

/* メッセージリストへの追加 */
static void add_message_list(client_info *client, char *_mess, message_info **prev_message)
{
  message_info *_message = malloc(sizeof(message_info));
  int len = snprintf(_message->message, BUFLEN, "MESG [%s] %s", client->name, _mess);
  _message->message[len] = '\0';
  _message->sender_sock = client->sock;
  if(message == NULL){
    message = _message;
    message->next = NULL;
    (*prev_message) = message;
  }else{
    (*prev_message)->next = _message;
    _message->next = NULL;
    *prev_message = _message;
  }
}

/* クライアントの削除 */
static void delete_member(int client_sock)
{
  client_info *client = NULL;
  client_info *prev = NULL;
  client = Client;
  while(client != NULL){
    if(client->sock == client_sock){
      if(prev == NULL){
        Client = client->next;
      }else{
        prev->next = client->next;
      }
      close(client->sock);
      //printf("Client[%s] disconnected.\n", client->name);
      free(client);
      break;
    }
    prev = client;
    client = client->next;
  }
}

/* クライアントへのメッセージの送信 */
static void send_message()
{
  int client_id;
  int len;
  message_info *prev_message = NULL;
#ifdef DEBUG
  printf("Send Message Start.\n");
  printf("Message:%s\n", message->message);
#endif
  show_message_main(&cht_arg->win_main, (message->message)+5);
  while(message != NULL){
    len=strlen(message->message);
    /* メッセージを送信する */
    client_info *client = NULL;
    client = Client;
    while(client != NULL){
      if(client->sock == message->sender_sock){
        client = client->next;
        continue;
      }
      if(send(client->sock, message->message, len, MSG_NOSIGNAL)==-1){
        if(errno == EPIPE){
          client_info *_delete = client;
          client = client->next;
          delete_member(_delete->sock);
          continue;
        }else{
          exit_errmesg("send()");
        }
      };
      client = client->next;
    }
    prev_message = message;
    message = message->next;
    free(prev_message);
  }
}

/* サーバーからクライアントへのメッセージの送信 */
void send_message_from_server(char* msg)
{
  int len;
  len = strlen(msg);
  client_info *client = NULL;
  client = Client;
  while(client != NULL){
    if(send(client->sock, msg, len, MSG_NOSIGNAL)==-1){
      if(errno == EPIPE){
        client_info *_delete = client;
        client = client->next;
        delete_member(_delete->sock);
        continue;
      }else{
        exit_errmesg("send()");
      }
    };
    client = client->next;
  }
}

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
  wprintw(*win_main, "%s\n", message);
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

/* UDPクライアントの初期化(broadcast想定) */
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

/* ブロードキャストアドレス情報をsockaddr_in構造体に格納する */
void set_sockaddr_in_broadcast(struct sockaddr_in *server_adrs, 
		               in_port_t port_number )
{
  /* ブロードキャストアドレスの情報をsockaddr_in構造体に格納する */
  memset(server_adrs, 0, sizeof(struct sockaddr_in));
  server_adrs->sin_family = AF_INET;
  server_adrs->sin_port = htons(port_number);
  server_adrs->sin_addr.s_addr = htonl(INADDR_BROADCAST);
}

/* クライアントの初期化（IP直接設定） */
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