#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define PROXYPORT 8080  /* プロキシサーバのポート番号 */
#define BUFSIZE 1024    /* バッファサイズ */

int main(int argc, char const *argv[])
{
  struct hostent *server_host;
  struct sockaddr_in server_adrs;

  int tcpsock;
  int proxyport = 0;
  int serverport = 80;

  //char proxyname[] = "proxy.cis.kit.ac.jp"; /*プロキシサーバ */
  char* proxyname;
  char k_buf[BUFSIZE], s_buf[BUFSIZE], r_buf[BUFSIZE];
  int strsize;

  char* serverName = argv[1];
  
  if(argc >= 3){
    proxyname = argv[2];
    proxyport = atoi(argv[3]);
  }

  /* サーバ名をアドレス(hostent構造体)に変換する */
  if(proxyport != 0){
    server_host =  gethostbyname(proxyname) ;
  }else{
    server_host = gethostbyname(serverName);
  }
  if(server_host == NULL){
    fprintf(stderr,"gethostbyname()");
    exit(EXIT_FAILURE);
  }

  /* サーバの情報をsockaddr_in構造体に格納する */
  memset(&server_adrs, 0, sizeof(server_adrs));
  server_adrs.sin_family = AF_INET;
  if(proxyport != 0)server_adrs.sin_port =  htons(proxyport);
  else server_adrs.sin_port =  htons(serverport);
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
  printf("please enter HTTP command: ");

  if(proxyport == 0){
    char command[] = "GET / HTTP/1.1\r\nHost: \r\n";
    strcat(command, server_host->h_addr_list[0]);
    strsize = strlen(command);
    if(send(tcpsock, command, strsize, 0) == -1){
      printf("Error; can't send HTTP command");
      exit(EXIT_FAILURE);
    }
  }else{
    char command[] = "GET ";
    strcat(command, serverName);
    strcat(command, " HTTP/1.1\r\n");
    strsize = strlen(command);
    if(send(tcpsock, command, strsize, 0) == -1){
      printf("Error; can't send HTTP command");
      exit(EXIT_FAILURE);
    }
  }
  send(tcpsock, "\r\n", 2, 0); /* HTTPのメソッド（コマンド）の終わりは空行 */
  printf("Send commands\n");
  /* サーバから文字列を受信する */
  if((strsize= recv(tcpsock, r_buf, BUFSIZE-1, 0) ) == -1){
    fprintf(stderr,"recv()");
    exit(EXIT_FAILURE);
  }
  r_buf[strsize] = '\0';
 
  /* 受信した文字列を画面に書く */
  printf("%s",r_buf);

  close(tcpsock);             /* ソケットを閉じる */
  exit(EXIT_SUCCESS);
}
