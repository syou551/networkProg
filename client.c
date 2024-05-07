///result of Running
///1. >./client https://www.cis.kit.ac.jp
///   Server: Apache
///   Content-Length: 210
///2. >./client http://www.google.com
///   Server: gws
///   Content-Length isn't included in response
///3. >./client https://syou551.dev
///   Server: cloudflare
///   Content-Length isn't included in response

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define SERVERPORT 80  /* プロキシサーバのポート番号 */
#define BUFSIZE 1024    /* バッファサイズ */

int main(int argc, char const *argv[])
{
  struct hostent *server_host;
  struct sockaddr_in server_adrs;

  int tcpsock;
  int proxyport = 0;

  char* res_tok;
  char* proxyname;
  char k_buf[BUFSIZE], s_buf[BUFSIZE], r_buf[BUFSIZE];
  int strsize,index=0;

  if(argc >= 3){
    proxyport = atoi(argv[3]);
  }

  /* サーバ名をアドレス(hostent構造体)に変換する */
  if(strstr(argv[1],"https")) {
    index = 8;
  }
  else if(strstr(argv[1],"http")){
    index = 7;
  }
  //printf("%s\n",argv[1]+index);
  if(proxyport != 0){
    server_host =  gethostbyname(argv[2]) ;
  }else{
    server_host = gethostbyname(argv[1]+index);
  }
  if(server_host == NULL){
    fprintf(stderr,"gethostbyname()");
    exit(EXIT_FAILURE);
  }

  /* サーバの情報をsockaddr_in構造体に格納する */
  memset(&server_adrs, 0, sizeof(server_adrs));
  server_adrs.sin_family = AF_INET;
  if(proxyport != 0)server_adrs.sin_port =  htons(proxyport);
  else server_adrs.sin_port =  htons(SERVERPORT);
  memcpy(&server_adrs.sin_addr, server_host->h_addr_list[0], server_host->h_length);

  /* ソケットをSTREAMモードで作成する */
  if((tcpsock =  socket(PF_INET, SOCK_STREAM, 0)  ) == -1){
    fprintf(stderr,"socket()");
    exit(EXIT_FAILURE);
  }

  /* ソケットにサーバの情報を対応づけてサーバに接続する */
  if( connect(tcpsock, (struct sockaddr *)&server_adrs, sizeof(server_adrs)) == -1){
    fprintf(stderr,"connect");
    exit(EXIT_FAILURE);
  }

  if(proxyport == 0){
    char command[BUFSIZE] = "GET / HTTP/1.1\r\nHost: ";
    strcat(command, argv[1]+index);
    strcat(command, "\r\n");
    strsize = strlen(command);
    if(send(tcpsock, command, strsize, 0) == -1){
      printf("Error; can't send HTTP command");
      exit(EXIT_FAILURE);
    }
  }else{
    char command[BUFSIZE] = "GET ";
    strcat(command, argv[1]);
    strcat(command, " HTTP/1.1\r\n");
    strsize = strlen(command);
    if(send(tcpsock, command, strsize, 0) == -1){
      printf("Error; can't send HTTP command");
      exit(EXIT_FAILURE);
    }
  }
  send(tcpsock, "\r\n", 2, 0); /* HTTPのメソッド（コマンド）の終わりは空行 */

  /* サーバから文字列を受信する */
  if((strsize= recv(tcpsock, r_buf, BUFSIZE-1, 0) ) == -1){
    fprintf(stderr,"recv()");
    exit(EXIT_FAILURE);
  }
  r_buf[strsize] = '\0';

  char* tok = "\r\n";
  res_tok = strtok(r_buf,tok);
  int flag[2] = {0,0};
  while(res_tok != NULL){
    if(strstr(res_tok,"Server:")){
      printf("%s\n",res_tok);
      flag[0] = 1;
    }
    else if(strstr(res_tok, "Content-Length:")){
      printf("%s\n",res_tok);
      flag[1] = 1;
    }
    res_tok = strtok(NULL,tok);
  }
  if(flag[0] != 1)printf("Server isn't included in response\n"); 
  if(flag[1] != 1)printf("Content-Length isn't included in response\n");
  close(tcpsock);             /* ソケットを閉じる */
  exit(EXIT_SUCCESS);
}
