#include "../mynet/mynet.h"
#include <sys/wait.h>
#include <pthread.h>

#define BUFSIZE 500   /* バッファサイズ */
#define PRCS_LIMIT 10 /* プロセス数制限 */

void * echo_thread(void *arg);
void echo_fork(int sock_listen, int thrnum);
int service(int sock_accepted);

/* スレッド関数の引数 */
struct myarg {
  int sock; /*listenしているソケット*/
  int id;      /* スレッドの通し番号 */
};

int main(int argc, char *argv[])
{
  int port_number, thread_number;
  int sock_listen;
  int i,mode;
  struct myarg *tharg;
  pthread_t tid;

  /* 引数のチェックと使用法の表示 */
  if( argc != 4 ){
    fprintf(stderr,"Usage: %s Port_number Num_of_thread mode\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  port_number = atoi(argv[1]); /* 引数の取得 */
  thread_number = atoi(argv[2]);
  mode = atoi(argv[3]);

  /* サーバの初期化 */
  sock_listen = init_tcpserver(port_number, 5);

  if(mode == 0){
    echo_fork(sock_listen, thread_number);
  }else if(mode == 1){
    for(i=0; i<thread_number; i++){
      /* スレッド関数の引数を用意する */
      /*これは独自の構造体*/
      if( (tharg = (struct myarg *)malloc(sizeof(struct myarg))) == NULL ){
        exit_errmesg("malloc()");
      }

      tharg->sock = sock_listen;
      tharg->id = i;

        /* スレッドを生成する */
      if( pthread_create(&tid, NULL, echo_thread, (void *)tharg) != 0 ){
        exit_errmesg("pthread_create()");
      }
    }
    pthread_exit(NULL);
  }else{
    fprintf(stderr,"ERROR: Invaliie mode. mode 0:fork 1:thread\n");
    exit(EXIT_FAILURE);
  }
}

void echo_fork(int sock_listen, int thrnum)
{
  int sock_accepted, n_process = 0;
  pid_t child;
  int strsize;
  char buf[BUFSIZE];

  for(int i=0;i<thrnum;i++){

    //ここでプログラムが実行状態そのままで二つに分裂する
    //以下のifはそれぞれのプロセスで処理される
    if( (child=fork()) == 0 ){
      /* Child process */
      int n = 1;
      while(n){
        /* クライアントの接続を受け付ける */
        sock_accepted = accept(sock_listen, NULL, NULL);
        /*processの使用状況調査用*/
        char no[20] = "Process id.";
        char id[10];
        sprintf(id, "%d", getpid());
        strcat(no,id);
        strcat(no,"\r\n");
        if(send(sock_accepted, no, strlen(no), 0) == -1 ){
            fprintf(stderr,"send()");
            exit(EXIT_FAILURE);
        }
        n = service(sock_accepted);

        close(sock_accepted);
      }
      //デバッグ用コマンドkillで強制終了
      exit(EXIT_SUCCESS);
    }
    else if(child>0){
      /* parent's process */
      n_process++;
      printf("Client is maked.[%d]\n", child);
      close(sock_accepted);
    }
    else{
      /* fork()に失敗 */
      close(sock_listen);
      exit_errmesg("fork()");
    }

    /* ゾンビプロセスの回収 */
    if( n_process == PRCS_LIMIT ){
      child= wait(NULL); /* 制限数を超えたら 空きが出るまでブロック */
      n_process--;
    }

    while( (child=waitpid(-1, NULL, WNOHANG ))>0 ){
      n_process--;
    }

  }
}

/* スレッドの本体 */
void * echo_thread(void *arg)
{
  struct myarg *tharg;
  char r_buf[BUFSIZE], s_buf[BUFSIZE];
  int strsize;
  int sock_accepted;
  int flag = 1;

  tharg = (struct myarg *)arg;
  pthread_detach(pthread_self()); /* スレッドの分離(終了を待たない) */
  /* クライアントの接続を受け付ける */
  while(1){
    sock_accepted = accept(tharg->sock, NULL, NULL);
    /*threadの使用状況調査用*/
    char id[20] = "Thread id.";
    char no[10];
    sprintf(no, "%d", tharg->id);
    strcat(id,no);
    strcat(id,"\r\n");
    if(send(sock_accepted, no, strlen(no), 0) == -1 ){
        fprintf(stderr,"send()");
        exit(EXIT_FAILURE);
    }
    service(sock_accepted);

    close(sock_accepted);
  }
  exit(EXIT_SUCCESS);
}

/* 文字列の最後の改行を削除する */
char * chop_nl(char *s)
{
int len;
len = strlen(s); // sが\0で終端されていることが前提。strnlen()を使う方が安全。

if( len>0 && (s[len-1]=='\n' || s[len-1]=='\r') ){
s[len-1] = '\0';
if( len>1 && s[len-2]=='\r'){
s[len-2] = '\0';
}
}

return s;
}


int service(int sock_accepted){
    int strsize,flag = 1;
    //パスワード認証
    char password[] = "Amagasaki2022";
    if(send(sock_accepted, "Password: ", 9, 0) == -1 ){
        fprintf(stderr,"send()");
        exit(EXIT_FAILURE);
    }
    char* pbuf = malloc(100);
    if((strsize=recv(sock_accepted, pbuf, BUFSIZE, 0)) == -1){
        fprintf(stderr,"recv()");
        exit(EXIT_FAILURE);
    }
    pbuf = chop_nl(pbuf);
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
        rbuf = chop_nl(rbuf);
        if(!strcmp(rbuf,"exit")) break;
        else if(!strcmp(rbuf,"kill")) { //デバッグ時の強制終了コマンド
          return 0;
        }else if(!strcmp(rbuf,"list")){
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
    }
    return 1;
}