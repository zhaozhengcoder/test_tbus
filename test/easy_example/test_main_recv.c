#include <stdio.h>
#include <stdlib.h>
#include "../../src/mytbus/mytbus.h" //需要的头文件
#include <string.h>
#include <string>
#include <sys/time.h>

const int tbus_shmkey = 2681;
string src_addr = "2.6.8.1";
string dest_addr = "0";

timeval getCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv;
}

int example_recv_msg_api()
{
    mytbus_init(tbus_shmkey);
    int msg_num = 0;
    int recv_msg_num = 3;

    while (msg_num < recv_msg_num)
    {
        char *recv_msg = new char[20];
        int recv_msg_len = 0;
        int ret = mytbus_recv(src_addr, dest_addr, recv_msg, recv_msg_len);
        if (ret == -1)
        {
            printf("tbus is empty \n");
        }
        else
        {
            printf("recv_msg : %s \n", recv_msg);
            msg_num++;
        }
    }
    return 0;
}

int main()
{
    timeval begintime = getCurrentTime();
    example_recv_msg_api();
    timeval endtime = getCurrentTime();

    printf("begin time sec : %ld , and usec %ld \n", begintime.tv_sec, begintime.tv_usec);
    printf("end time   sec : %ld , and usec %ld \n", endtime.tv_sec, endtime.tv_usec);
    return 0;
}
