# 概要
ネットワークプログラミング用レポジトリ
作成したCソースファイルを主に保存する

# 内容
* client.c - TCPソケット等を用いたGET/HEADリクエストプログラム
* echoClient.c - echoするネットワークプログラム
* server.c - echo用サーバープログラム（erchoClientのサーバーサイド）

第1引数に対象のWebサーバーのURL,第2引数ProxyURL(optional),第3引数ProxyPort番号(Optional)
