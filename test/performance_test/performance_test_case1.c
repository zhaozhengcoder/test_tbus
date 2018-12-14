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


//发送一条消息，之后读取一条消息，并且检查消息的正确性
bool case1()
{
    char send_msg[] ="hello world";
    int msg_len = strlen(send_msg)+1;
    mytbus_send(send_src_addr, send_dest_addr, send_msg, msg_len);

    char recv_msg[20];
    int recv_msg_len = 0;
    
    int ret = mytbus_recv(recv_src_addr, recv_dest_addr, recv_msg, recv_msg_len);
    if (ret == -1)
    {
        return false;
    }
    else
    {
        if (strcmp(send_msg, recv_msg) == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

//发送方发送1000条消息，然后接受方接受1000条消息
bool case2()
{
    int msg_num = 1000;
    char send_msg[] ="hello world";
    for(int i=0;i<msg_num;i++)
    {
        int msg_len = strlen(send_msg)+1;
        mytbus_send(send_src_addr, send_dest_addr, send_msg, msg_len);
    }
    for(int i=0;i<msg_num;i++)
    {
        char recv_msg[20];
        int recv_msg_len = 0;
        int ret = mytbus_recv(recv_src_addr, recv_dest_addr, recv_msg, recv_msg_len);
        if (ret == -1)
        {    
            return false;
        }
        else
        {
            if (strcmp(send_msg, recv_msg) == 0)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return true;
}

int main()
{
    mytbus_init(tbus_shmkey);

    if(case1())
    {cout<<"[case 1] pass"<<endl;}
    else
    {cout<<"[case 1] fail"<<endl;}
    
    if(case2())
    {cout<<"[case 2] pass"<<endl;}
    else
    {cout<<"[case 2] fail"<<endl;}

    return 0;
}