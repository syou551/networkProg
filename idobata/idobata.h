#ifndef IDOBATA_H_
#define IDOBATA_H_

#include "../mynet/mynet.h"
#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <curses.h>
#include <ncurses.h>
#include <locale.h>

#define BUFSIZE 1024 /* バッファサイズ */
#define DEFAULT_PORT 50001       /* ポート番号 */

/* サーバメインルーチン */
void server_main(int port_number, int n_client);

/* クライアントメインルーチン */
void client_main(struct sockaddr_in server_adrs, char *user_name, in_port_t port);

/* クライアントの初期化 */
void init_client(int sock_listen, int n_client);

/* Accept関数(エラー処理つき) */
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);

/* 送信関数(エラー処理つき) */
int Send(int s, void *buf, size_t len, int flags);

/* 受信関数(エラー処理つき) */
int Recv(int s, void *buf, size_t len, int flags);

/* ウィンドウの初期化 */
void init_window(WINDOW **win_main, WINDOW **win_sub);

/* メインウィンドウにメッセージを表示 */
void show_message_main(WINDOW **win_main, char *message);

/* サブウィンドウから入力を受け取る */
int input_message_sub(WINDOW **win_sub, char *message, int S_BUFSIZE);

void set_sockaddr_in_broadcast(struct sockaddr_in *server_adrs, in_port_t port_number );

int init_udpclient_broadcast(int broadcast_sw);

int init_tcpclient_ip(struct sockaddr_in server_adrs, in_port_t serverport);

char *chop_nl(char *s);

#endif /* IDOBATA_H_ */