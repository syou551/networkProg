#include "../mynet/mynet.h"

#define BUFSIZE 500   /* バッファサイズ */

int main(int argc, char* argv[])
{
  int sock_listen, sock_accepted;
  int strsize,flag=1;
  int port=50000;

  if(argc<2){
    printf("You didn't input port. Use default port 50000 ...\n");
  }else{
    port=atoi(argv[1]);
    if(*argv[1] != '0' && port == 0){
        printf("Error: Invalid port! Please input available port!\n");
        return(-1);
    }
  }

  /* 待ち受け用ソケットをSTREAMモードで作成する */
  sock_listen = init_tcpserver(port, 5);

  /* クライアントの接続を受け付ける */
  while(1){
    sock_accepted = accept(sock_listen, NULL, NULL);

    //パスワード認証
    char password[] = "Amagasaki2022\r\n";
    if(send(sock_accepted, "Password: ", 9, 0) == -1 ){
        fprintf(stderr,"send()");
        exit(EXIT_FAILURE);
    }
    char* pbuf = malloc(100);
    if((strsize=recv(sock_accepted, pbuf, BUFSIZE, 0)) == -1){
        fprintf(stderr,"recv()");
        exit(EXIT_FAILURE);
    }
    if(strcmp(pbuf,password)){
        if(send(sock_accepted, "Invalid passsword\r\n", 18, 0) == -1 ){
            fprintf(stderr,"send()");
            exit(EXIT_FAILURE);
        }
        flag=0;
    }
    free(pbuf);

    while(flag){
        if(send(sock_accepted, ">", 1, 0) == -1 ){
            fprintf(stderr,"send()");
        exit(EXIT_FAILURE);
        }
        char* rbuf = malloc(100);
        char* buf = malloc(BUFSIZE);
        /* 文字列をクライアントから受信する */
        if((strsize=recv(sock_accepted, rbuf, BUFSIZE, 0)) == -1){
            fprintf(stderr,"recv()");
            exit(EXIT_FAILURE);
        }

        if(!strcmp(rbuf,"exit\r\n")) break;
        else if(!strcmp(rbuf,"list\r\n")){
            FILE *fp;
            if((fp = popen("ls ~/work","r"))== NULL){
                strcpy(buf, "list cmd internal Error\r\n");
                strsize = strlen(buf);
            }else{
                while(fgets(buf, sizeof(buf),fp)!=NULL){
                    strsize = strlen(buf);
                    if(send(sock_accepted, buf, strsize, 0) == -1 ){
                        fprintf(stderr,"send()");
                        exit(EXIT_FAILURE);
                    }
                }
                pclose(fp);
                strsize = 0;
            }
        }else if(!strstr(rbuf,"type")==NULL){
            FILE *fp;
            char cmd[BUFSIZE] = "cat ~/work/";
            
            if(strstr(rbuf,"/")!=NULL){
                printf("Error\n");
                strcpy(buf, "Error:No supported filename!\r\n");
                strsize = strlen(buf);
            }else{
                strcat(cmd,rbuf+5);
                strsize = strlen(cmd);
                cmd[strsize-2] = '\0';
                if((fp=popen(cmd,"r"))==NULL){
                    printf("Error\n");
                    strcpy(buf, "Error:Not found this name file\r\n");
                    strsize = strlen(buf);
                }else{
                    while(fgets(buf, sizeof(buf),fp)!=NULL){
                        strsize = strlen(buf);
                        if(send(sock_accepted, buf, strsize, 0) == -1 ){
                            fprintf(stderr,"send()");
                            exit(EXIT_FAILURE);
                        }
                    }
                    pclose(fp);
                    strsize = 0;
                }
            }
        }else{
            printf("%s",rbuf);
            strcpy(buf, "No Supported Command\r\n");
            strsize = strlen(buf);
        }

        /* 文字列をクライアントに送信する */
        if(strsize == 0) {}
        else if(send(sock_accepted, buf, strsize, 0) == -1 ){
            fprintf(stderr,"send()");
            exit(EXIT_FAILURE);
        }
        free(rbuf);
        free(buf);
    };

    close(sock_accepted);
    flag = 1;
  }
  exit(EXIT_SUCCESS);
}