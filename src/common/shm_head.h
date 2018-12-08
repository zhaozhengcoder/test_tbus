#ifndef shm_head_h
#define shm_head_h
#include <string>
using namespace std;

#define max_channel_num 128
#define ADDRESS_STR_LEN 16

struct CHANNELS_PAIR 
{
    char address1_str[ADDRESS_STR_LEN];
    char address2_str[ADDRESS_STR_LEN];
    int address1;
    int address2;
    int channel_index;
};

//所有的偏移地址，都表示的是相对于共享内存的起始偏移
struct SHM_HEAD
{
    int channels_num;
    CHANNELS_PAIR pair_array[max_channel_num];
    int channels_config_begin_addr[max_channel_num];     
    int channels_config_end_addr[max_channel_num];    
}; 

struct QUEUE_PAIR
{
    int address;
    int queue_index;   //存放的是address作为地址，对应的发送的queue的index
};

struct SHM_CHANNELS_CONFIG
{
    QUEUE_PAIR pair;          //根据pair来找到queue_begin_addr对应的下标是0还是1
    int queue_begin_addr[2];
    int queue_end_addr[2];
    int queue_head[2];        //表示的在循环队列上面的偏移，而不是相对于共享内存上面的偏移呐!
    int queue_tail[2];
    int msg_num[2];
    int queue_channel_size[2];
};

#endif