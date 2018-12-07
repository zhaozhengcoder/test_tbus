#ifndef MYTBUS_H_
#define MYTBUS_H_

#include <string>
using namespace std;

int mytbus_init(int tbus_shmkey);
int mytbus_send(string src_addr,string dest_addr,char * msg,int msg_len);

//从tbus里面收到的消息 是没有头部长度的，头部的长度已经存放到了 msg_len的字段里面
int mytbus_recv(string src_addr,string & dest_addr,char * msg,int & msg_len);

#endif 
