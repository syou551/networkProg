#include "idobata.h"

#define RETRY 3 /* 再試行回数 */
#define TIMEOUT_SEC 5 /* タイムアウト秒 */
#define INIT_PORT 60000 /* ポート番号 */

int main(int argc, char *argv[])
{
  struct sockaddr_in server_adrs;
  struct sockaddr_in from_adrs;
  int sock, from_len;
  int broadcast_sw = 1;
  in_port_t port = DEFAULT_PORT;
  char helo[] = "HELO";
  char here[] = "HERE";
  char s_buf[BUFSIZE], r_buf[BUFSIZE];
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
  if(strlen(argv[1])>15){
    fprintf(stderr, "user_anme too long! 15 or less");
    exit(1);
  }
  /* ブロードキャストアドレスを設定 */
  set_sockaddr_in_broadcast(&server_adrs, port);

  /* ソケットをSTREAMモードで作成する */
  if((sock = init_udpclient_broadcast(broadcast_sw)) == -1){
    exit(EXIT_FAILURE);
  }

  FD_ZERO(&mask);
  FD_SET(sock, &mask);

  /* サーバから文字列を受信 */
  for(int i=0;i<RETRY;i++){
    Sendto(sock, helo, strlen(helo), 0, (struct sockaddr *)&server_adrs, sizeof(server_adrs));
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
      pthread_exit(EXIT_SUCCESS);
    }else{
        printf("Received: %s\n", r_buf);
        printf("Incorrect format response receive!\n");
    }
  }

  close(sock);
  printf("Start as Server!\n");
  //サーバーのメインループ
  server_main(argv[1],port);
  pthread_exit(EXIT_SUCCESS);
}