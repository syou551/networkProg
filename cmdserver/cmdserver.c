#include "../mynet/mynet.h"

#define PORT 50000         /* ポート番号 ←適当に書き換える */
#define BUFSIZE 1000   /* バッファサイズ */

int main()
{
  int sock_listen, sock_accepted;

  char buf[BUFSIZE];
  int strsize;

  /* 待ち受け用ソケットをSTREAMモードで作成する */
  sock_listen = init_tcpserver(PORT, 5);

  /* クライアントの接続を受け付ける */
  while(1){
    sock_accepted = accept(sock_listen, NULL, NULL);

    //close(sock_listen);

    while(1){
        if(send(sock_accepted, ">", 1, 0) == -1 ){
            fprintf(stderr,"send()");
        exit(EXIT_FAILURE);
        }
        char rbuf[BUFSIZE];
        /* 文字列をクライアントから受信する */
        if((strsize=recv(sock_accepted, rbuf, BUFSIZE, 0)) == -1){
        fprintf(stderr,"recv()");
        exit(EXIT_FAILURE);
        }
        printf("%s",rbuf);
        if(strcmp(rbuf,"exit\r\n")==0) break;
        else if(strcmp(rbuf,"list\r\n")==0){
            FILE *fp;
            if((fp = popen("ls ~/Desktop/work","r"))== NULL){
                strcpy(buf, "list cmd internal Error\n");
                strsize = strlen(buf);
            }else{
                char str[BUFSIZE]="";
                while(fgets(buf, sizeof(buf),fp)!=NULL){
                    printf("%s", buf);
                    strcat(str, buf);
                    strsize = strlen(str);
                }
                pclose(fp);
                strcpy(buf, str);
            }
        }else if(strcmp(rbuf,"type\r\n")==0){
            FILE *fp;
            if((fp=popen("cat ~/Desktop/work/test.txt","r"))==NULL){
                strcpy(buf, "type cmd internal Error\n");
                strsize = strlen(buf);
            }else{
                char str[BUFSIZE]="";
                while(fgets(buf, sizeof(buf),fp)!=NULL){
                    printf("%s", buf);
                    strcat(str, buf);
                    strsize = strlen(str);
                }
                pclose(fp);
                strcpy(buf, str);
            }

        }else{
            strcpy(buf, "No Supported Command;\n");
            strsize = strlen(buf);
        }

        /* 文字列をクライアントに送信する */
        if(send(sock_accepted, buf, strsize, 0) == -1 ){
            fprintf(stderr,"send()");
        exit(EXIT_FAILURE);
        }
    };

    close(sock_accepted);

  }
  exit(EXIT_SUCCESS);
}