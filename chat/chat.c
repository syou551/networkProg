/*
工夫した点，苦労した点は各ソースコードの冒頭にそれぞれ記述
ここにはまとめて全体で工夫したこと，苦労したことを記述
・工夫した点
	- 他のクライアントの接続待ちなどの時にサーバーからメッセージを送信し，状態を確認できるようにした
	- サーバーがダウンした時，クライアントはメッセージを表示して終了するようにした
	- クライアントが全員接続を切断した場合，サーバーが自動的に終了するようにした
	- クライアントでメッセージの入力，受信したメッセージが誰からのものか，自分のメッセージがどれかというのが
	  一目でわかるようにCursesによる画面制御を実装した
・苦労した点
	- cursesによる画面制御を実装する際，画面の初期化やメッセージの表示，入力などの処理が複雑であった点
*/
#include "chat.h"
#include "mynet.h"
#include <stdlib.h>
#include <unistd.h>

#define SERVER_LEN 256     /* サーバ名格納用バッファサイズ */
#define DEFAULT_PORT 50000 /* ポート番号既定値 */
#define DEFAULT_NCLIENT 3  /* 省略時のクライアント数 */
#define DEFAULT_MODE 'C'   /* 省略時はクライアント */

extern char *optarg;
extern int optind, opterr, optopt;

//main関数
//コマンド引数よりサーバーモードかクライアントモードかを判断し，それぞれの関数を呼び出す
int main(int argc, char *argv[])
{
  int port_number=DEFAULT_PORT;
  int num_client =DEFAULT_NCLIENT;
  char servername[SERVER_LEN] = "localhost";
  char mode = DEFAULT_MODE;
  int c;

  /* オプション文字列の取得 */
  opterr = 0;
  while( 1 ){
	  c = getopt(argc, argv, "SCs:p:c:h");
	  if( c == -1 ) break;
	  
	  switch( c ){
	  case 'S' :  /* サーバモードにする */
	    mode = 'S';
	    break;

	  case 'C':   /* クライアントモードにする */
	    mode = 'C';
	    break;

	  case 's' :  /* サーバ名の指定 */
	    snprintf(servername, SERVER_LEN, "%s", optarg);
	    break;

	  case 'p':  /* ポート番号の指定 */
	    port_number = atoi(optarg);
	    break;

	  case 'c':  /* クライアントの数 */
	    num_client = atoi(optarg);
	    break;

	  case '?' :
	    fprintf(stderr,"Unknown option '%c'\n", optopt );
	  case 'h' :
	    fprintf(stderr,"Usage(Server): %s -S -p port_number -c num_client\n", argv[0]);
	    fprintf(stderr,"Usage(Client): %s -C -s server_name -p port_number\n", argv[0]);
	    exit(EXIT_FAILURE);
	    break;
	  }
  }

  switch(mode){

  case 'S':
    chat_server(port_number, num_client);
    break;
  case 'C':
    chat_client(servername, port_number);
    break;

  }

  exit(EXIT_SUCCESS);
}
