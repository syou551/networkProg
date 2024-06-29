#include "idobata.h"

#define RETRY 3 /* 再試行回数 */
#define TIMEOUT_SEC 5 /* タイムアウト秒 */

int main(int argc, char *argv[])
{
  struct sockaddr_in server_adrs;
  struct sockaddr_in from_adrs;
  int sock, from_len;
  int broadcast_sw = 1;
  in_port_t port = DEFAULT_PORT;
  char helo[] = "HELO";
  char here[] = "HERE";
  char s_buf[1024], r_buf[BUFSIZE];
  int strsize;
  fd_set mask, readfds;
  struct timeval timeout;

  /* 引数のチェックと使用法の表示 */
  if( argc < 2 ){
    fprintf(stderr,"Usage: %s user_name [port: optional]\n", argv[0]);
    exit(1);
  }

  if(argc == 3){
    port = (in_port_t)atoi(argv[2]);
  }
  /* ブロードキャストアドレスを設定 */
  set_sockaddr_in_broadcast(&server_adrs, port);

  /* ソケットをSTREAMモードで作成する */
  if((sock = init_udpclient_broadcast(broadcast_sw)) == -1){
    exit(EXIT_FAILURE);
  }

  FD_ZERO(&mask);
  FD_SET(sock, &mask);

  Sendto(sock, helo, strlen(helo), 0, (struct sockaddr *)&server_adrs, sizeof(server_adrs));

  /* サーバから文字列を受信 */
  for(int i=0;i<RETRY;i++){

    /* 受信データの有無をチェック */
    readfds = mask;
    timeout.tv_sec = TIMEOUT_SEC;
    timeout.tv_usec = 0;
    
    if( select( sock+1, &readfds, NULL, NULL,&timeout)==0 ){
      printf("Time out.\n");
      continue;
    }

    from_len = sizeof(from_adrs);
    strsize = Recvfrom(sock, r_buf, BUFSIZE-1, 0, (struct sockaddr *)&from_adrs, &from_len);
    r_buf[strsize] = '\0';
    if(strcmp(r_buf, here) == 0){
      close(sock);
      printf("Start as Cliant!\n");
      //クライアントのメインループ
      client_main(from_adrs, argv[1], port);
    }else{
        printf("Received: %s\n", r_buf);
        printf("Incorrect format response receive!\n");
    }
  }

  close(sock);
  printf("Can't find server! This program is going to finish.\n");
  //printf("Start as Server!\n");
  //サーバーのメインループ
  //server_main(port);

  exit(EXIT_SUCCESS);
}