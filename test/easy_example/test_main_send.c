#include <stdio.h>
#include <stdlib.h>
#include "../../src/mytbus/mytbus.h" 
#include <string.h>
#include <string>
#include <sys/time.h>

const int tbus_shmkey = 2681;
string dest_addr = "2.6.8.1";
string src_addr  = "2.6.8.2";

// 当前进程发送3条消息
int example_send_msg_api()
{
    mytbus_init(tbus_shmkey);

    char msg[] ="hello world";
    int msg_len = strlen(msg)+1;

    int send_msg_num = 3;  
    while(send_msg_num)
    {
        mytbus_send(src_addr, dest_addr, msg, msg_len);
        send_msg_num--;
    }
    return 0;
}

timeval getCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv;
}

int main()
{
    timeval begintime = getCurrentTime();
    example_send_msg_api();
    timeval endtime = getCurrentTime();

    printf("begin time sec : %ld , and usec %ld \n",begintime.tv_sec,begintime.tv_usec);
    printf("end time   sec : %ld , and usec %ld \n",endtime.tv_sec,endtime.tv_usec);
    return 0; 
}
