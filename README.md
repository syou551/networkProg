# 概要
ネットワークプログラミング用レポジトリ
作成したCソースファイルを主に保存する

# 内容
* mynet - mynetライブラリ
* client.c - TCPソケット等を用いたGET/HEADリクエストプログラム
    > 第1引数に対象のWebサーバーのURL,第2引数ProxyURL(optional),第3引数ProxyPort番号(Optional)

* echoClient.c - echoするネットワークプログラム
* server.c - echo用サーバープログラム（erchoClientのサーバーサイド）
* client_curses.c - Cursesを用いたサンプルプログラム
* cmdserver - 課題2のコマンドライクなサーバープログラム
    > listenするポートを起動時に指定し，クライアント側はtelnetなどで接続

* thread - threadを用いたecho用サーバープログラムサンプル
    > listenポートを起動時に指定

* fork - forkを用いたechoプログラム
* threadserver - 課題5のthread/forkを用いたcmdserver
* udpecho - udp ver.のechoプログラム
* signal - signalを用いたタイムアウトを実装したecho
* select - selectを用いたquizプログラム
* chat - 課題3のチャットプログラム
* broadcastecho - ブロードキャストによる自動接続を行うechoプログラム
* idobata - 課題5のチャットプログラム
    > usage: ./idobata user_name [port_number:option]