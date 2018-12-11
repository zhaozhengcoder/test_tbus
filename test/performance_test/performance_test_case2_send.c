#include <stdio.h>
#include <stdlib.h>
#include "../../src/mytbus/mytbus.h" //需要的头文件
#include <string.h>
#include <string>
#include <sys/time.h>
#include <iostream>


const int tbus_shmkey = 2681;
string send_dest_addr = "2.6.8.1";
string send_src_addr  = "2.6.8.2";

string recv_src_addr = "2.6.8.1";
string recv_dest_addr = "2.6.8.2";

using namespace std;

timeval getCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv;
}


void send_thread(int msg_num)
{
    int send_msg_num = 0;
    while(send_msg_num < msg_num)
    {
        char send_msg[] ="hello world";
        int msg_len = strlen(send_msg)+1;
        mytbus_send(send_src_addr, send_dest_addr, send_msg, msg_len);
        send_msg_num++;
    }
    cout<<"[send thread] send msg num "<<send_msg_num<<endl;
}

int main(int argc,char * argv[])
{
    mytbus_init(tbus_shmkey);
    if(argc<=1)
    {
        cout<<"usage : a.out [recv msg num]"<<endl;
    }
    else
    {
        long long  msg_num = atoi(argv[1]);
        send_thread(msg_num);
    }

}