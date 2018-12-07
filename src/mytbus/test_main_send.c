#include <stdio.h>
#include <stdlib.h>
#include "mytbus.h"
#include <string.h>
#include <string>
#include <sys/time.h>

int bench_send()
{
    int tbus_shmkey = 2681;
    mytbus_init(tbus_shmkey);
    string dest_addr = "2.6.8.1";
    string src_addr1 = "2.6.8.2";
    string src_addr2 = "2.6.8.3";

    char msg[] ="hello world";
    char msg2[] ="hello pipap";
    int msg_len = strlen(msg)+1;

    int times = 3;  
    while(times)
    {
        mytbus_send(src_addr1, dest_addr, msg, msg_len);
        mytbus_send(src_addr2, dest_addr, msg2, msg_len);
        times--;
    }
}

timeval getCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    //printf("sec : %d , and usec %d \n",tv.tv_sec,tv.tv_usec);
    return tv;
}

int main()
{
    timeval tv1 = getCurrentTime();
    bench_send();
    timeval tv2 = getCurrentTime();

    printf("start sec : %d , and usec %d \n",tv1.tv_sec,tv1.tv_usec);
    printf("end   sec : %d , and usec %d \n",tv2.tv_sec,tv2.tv_usec);
    return 0;  
}
