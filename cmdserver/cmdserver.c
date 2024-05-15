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
    if(argv[1] != '0' && port == 0){
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
        if(send(sock_accepted, "Invalid passsword\n", 18, 0) == -1 ){
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
            if((fp = popen("ls ~/Desktop/work","r"))== NULL){
                strcpy(buf, "list cmd internal Error\n");
                strsize = strlen(buf);
            }else{
                char* str=malloc(BUFSIZE);
                while(fgets(buf, sizeof(buf),fp)!=NULL){
                    strcat(str, buf);
                    strsize = strlen(str);
                }
                pclose(fp);
                strcpy(buf, str);
                free(str);
            }
        }else if(strsize >= 7){
            FILE *fp;
            char path[BUFSIZE] = "/Users/nakatahiroto/Desktop/work/";

            rbuf[strsize-2] = '\0';
            if(strstr(rbuf,"/")!=NULL){
                printf("Error\n");
                strcpy(buf, "Error:No supported filename!\n");
                strsize = strlen(buf);
            }else{
                strcat(path,rbuf+5);
                if((fp=fopen(path,"r"))==NULL){
                    printf("Error\n");
                    strcpy(buf, "Error:Not found this name file\n");
                    strsize = strlen(buf);
                }else{
                    char *str = malloc(BUFSIZE);
                    while(fgets(buf, sizeof(buf),fp)!=NULL){
                        strcat(str, buf);
                        strsize = strlen(str);
                        if(strstr(buf,"No such file or directory")!= NULL) break;
                    }
                    pclose(fp);
                    strcpy(buf, str);
                    free(str);
                }
            }
        }else{
            printf("%s",rbuf);
            strcpy(buf, "No Supported Command\n");
            strsize = strlen(buf);
        }

        /* 文字列をクライアントに送信する */
        if(send(sock_accepted, buf, strsize, 0) == -1 ){
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