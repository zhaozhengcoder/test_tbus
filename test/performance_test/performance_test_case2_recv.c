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


void recv_thread(int msg_num)
{
    char target_recv_msg[] = "hello world";
    int recv_msg_num = 0;
    while(recv_msg_num < msg_num)
    {
        char recv_msg[20];
        int recv_msg_len = 0;
        int ret = mytbus_recv(recv_src_addr, recv_dest_addr, recv_msg, recv_msg_len);
        if(ret != -1)
        {
            if (strcmp(recv_msg, target_recv_msg) == 0)
            {
                recv_msg_num++;
            }
        }
    }
    cout<<"[recv thread] recv msg num "<<recv_msg_num<<endl;
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
        long long msg_num = atoi(argv[1]);
        cout<<msg_num<<endl;
        recv_thread(msg_num);
    }
}