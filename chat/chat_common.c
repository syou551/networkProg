/*
・工夫した点
受信や送信のラッパ関数を用意することで，他のファイルでの記述を簡略化した．
*/
#include "../mynet/mynet.h"

/* accept のラッパ関数*/
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
  int r;
  if((r=accept(s,addr,addrlen))== -1){
    exit_errmesg("accept()");
  }
  return(r);
}

/* send のラッパ関数*/
int Send(int s, void *buf, size_t len, int flags)
{
  int r;
  if((r=send(s,buf,len,flags))== -1){
    exit_errmesg("send()");
  }
  return(r);
}

/* recv のラッパ関数*/
int Recv(int s, void *buf, size_t len, int flags)
{
  int r;
  if((r=recv(s,buf,len,flags))== -1){
    exit_errmesg("recv()");
  }
  return(r);
}