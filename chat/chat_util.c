#include "../mynet/mynet.h"
#include "chat.h"
#include <stdlib.h>
#include <sys/select.h>

#define NAMELENGTH 20 /* ログイン名の長さ制限 */
#define BUFLEN 500    /* 通信バッファサイズ */

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

void init_client(int sock_listen, int n_client)
{
  N_client = n_client;

  /* クライアント情報の保存用構造体の初期化 */
  if( (Client=(client_info *)malloc(N_client*sizeof(client_info)))==NULL ){
    exit_errmesg("malloc()");
  }
  /* クライアントのログイン処理 */
  Max_sd = client_login(sock_listen);

}

void chat_loop()
{
  for(;;){
    /* メッセージの受け取り */
    /* client が切断されると再びループの最初に戻る */
    if(receive_message()==0) {
      if(N_client==0) break;
      continue;
    };
    /* メッセージを他ユーザーへ送信 */
    send_message();

  }
  free(Client);
  printf("Clients had disconnected. Chat server terminated.\n");
}

static int client_login(int sock_listen)
{
  int client_id,sock_accepted;
  static char prompt[]="Input your name: ";
  static char wait_message[] = "Waiting for other clients...\n";
  static char start_message[] = "Start chat!\n";
  char loginname[NAMELENGTH];
  int strsize;

  for( client_id=0; client_id<N_client; client_id++){
    /* クライアントの接続を受け付ける */
    sock_accepted = Accept(sock_listen, NULL, NULL);
    printf("Client[%d] connected.\n",client_id);

    /* ログインプロンプトを送信 */
    Send(sock_accepted, prompt, strlen(prompt), 0);

    /* ログイン名を受信 */
    strsize = Recv(sock_accepted, loginname, NAMELENGTH-1, 0);
    loginname[strsize] = '\0';
    chop_nl(loginname);

    /* ユーザ情報を保存 */
    Client[client_id].sock = sock_accepted;
    strncpy(Client[client_id].name, loginname, NAMELENGTH);

    Send(sock_accepted, wait_message, strlen(wait_message), 0);
  }

  for( client_id=0; client_id<N_client; client_id++){
    Send(Client[client_id].sock, start_message, strlen(start_message), 0);
  }

  return(sock_accepted);

}

static int receive_message()
{
  fd_set mask, readfds;
  int client_id;
  int strsize;

  /* ビットマスクの準備 */
  FD_ZERO(&mask);
  for(client_id=0; client_id<N_client; client_id++){
    FD_SET(Client[client_id].sock, &mask);
  }
  /* 受信データの有無をチェック */
  readfds = mask;
  select( Max_sd+1, &readfds, NULL, NULL, NULL );

  for( client_id=0; client_id<N_client; client_id++ ){
    if( FD_ISSET(Client[client_id].sock, &readfds) ){
      Message = malloc(BUFLEN);
      strsize = Recv(Client[client_id].sock, Message, BUFLEN-1,0);
      if(strsize == 0){
        delete_member(client_id);
        return 0;
      }
      Message = chop_nl(Message);
      strsize = strlen(Message);
      Message[strsize]='\0';

      sender = client_id;
      FD_CLR(Client[client_id].sock, &mask);
      break;
    }
  }
  return 1;
}

static void delete_member(int client_id)
{
  int i;
  close(Client[client_id].sock);
  for(i=client_id; i<N_client-1; i++){
    Client[i] = Client[i+1];
  }
  N_client--;
  Max_sd = Client[N_client-1].sock;
  printf("Client[%d] disconnected.\n", client_id);
}

static void send_message()
{
  int client_id;
  int len;
  len=snprintf(Buf, BUFLEN, "From %s < %s\n", Client[sender].name, Message);
  /* メッセージを送信する */
  for(client_id=0; client_id<N_client; client_id++){
    if(client_id == sender) continue;
    Send(Client[client_id].sock, Buf, len, 0);
  }
  free(Message);
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