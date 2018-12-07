#include <stdio.h>
#include <stdlib.h>
#include "mytbus.h"
#include <string.h>
#include <string>
#include <sys/time.h>

timeval getCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    //printf("sec : %d , and usec %d \n",tv.tv_sec,tv.tv_usec);
    return tv;
}

int bench_recv()
{
    int tbus_shmkey = 2681;
    mytbus_init(tbus_shmkey);
    string src_addr = "2.6.8.1";
    //string src_addr = "0";
    //string dest_addr = "2.6.8.2";
    string dest_addr = "0";

    char * recv_msg = new char[20];
    int recv_msg_len=0;

    int msg_num = 0;
    int times = 3;  
    int empty_times = 0;
    int stop_empty_times = 50;

    while(msg_num<times)
    {
        int ret = mytbus_recv(src_addr, dest_addr, recv_msg, recv_msg_len);    //这里的地址是要相反的
        printf("debug dest addr [%s] \n",dest_addr.c_str());
        if(ret==-1)
        {
            printf("tbus is empty \n");
            empty_times++;
            if(empty_times>stop_empty_times)
            {
                printf("empty times : %d \n",empty_times);
                break;
            }
        }
        else
        {
            printf("recv_msg : %s \n",recv_msg);
            msg_num++;
        }
    }
    printf("msg_num : %d \n",msg_num);
    return 0;
}

int main()
{
    timeval tv1 = getCurrentTime();
    bench_recv();
    timeval tv2 = getCurrentTime();

    printf("start sec : %d , and usec %d \n",tv1.tv_sec,tv1.tv_usec);
    printf("end   sec : %d , and usec %d \n",tv2.tv_sec,tv2.tv_usec);

    return 0;
}
