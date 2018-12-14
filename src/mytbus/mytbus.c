#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <string.h>
#include <string>
#include "../common/shm_head.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <vector>
#include <unistd.h>
#include <getopt.h>

using namespace std;
const int max_msg_len = 256;

static SHM_HEAD *shm_head;
struct MSG
{
    int msg_len;
    char msg[max_msg_len];
};

int mytbus_init(int tbus_shmkey)
{
    tbus_shmkey = tbus_shmkey << 2;
    int shm_id = shmget(tbus_shmkey, 0, 0666 | IPC_CREAT);
    if (shm_id < 0)
    {
        printf("shmget error \n");
        return -1;
    }
    shm_head = (SHM_HEAD *)shmat(shm_id, NULL, 0);
    return 0;
}

//根据src喝dest的addr，获得channel的index
static int get_channel_index(string src_addr, string dest_addr)
{
    int channels_num = shm_head->channels_num;
    int min_addr = min(inet_addr(src_addr.c_str()), inet_addr(dest_addr.c_str()));
    int max_addr = max(inet_addr(src_addr.c_str()), inet_addr(dest_addr.c_str()));
    for (int i = 0; i < channels_num; i++)
    {
        if (shm_head->pair_array[i].address1 == min_addr && shm_head->pair_array[i].address2 == max_addr)
        {
            return shm_head->pair_array[i].channel_index;
        }
    }
    printf("get_channel_index error \n");
    return -1;
}

static void *get_channel_config_pointer(int channel_index)
{
    if (channel_index >= shm_head->channels_num)
    {
        printf("error \n");
        return NULL;
    }
    return static_cast<void *>(((char *)shm_head) + shm_head->channels_config_begin_addr[channel_index]);
}

//获得发送队列的index
static int get_send_queue_index(SHM_CHANNELS_CONFIG *channel_config_pointer, string src_addr)
{
    //获得发送src作为发送地址的，对应的index
    if (channel_config_pointer != NULL)
    {
        int queue_index;
        if (channel_config_pointer->pair.address == static_cast<int>(inet_addr(src_addr.c_str())))
            queue_index = 0;
        else
            queue_index = 1;
        return queue_index;
    }
    printf("channel_config_pointer == NULL \n");
    return -1;
}

//获得接收队列的index
static int get_recv_queue_index(const SHM_CHANNELS_CONFIG *channel_config_pointer, string src_addr)
{
    if (channel_config_pointer != NULL)
    {
        int queue_index;
        if (channel_config_pointer->pair.address == static_cast<int>(inet_addr(src_addr.c_str())))
            queue_index = 1;
        else
            queue_index = 0;
        return queue_index;
    }
    printf("channel_config_pointer == NULL \n");
    return -1;
}

//获得发送队列的剩余的空间
static int get_remain_send_size(int channel_index, string src_addr)
{
    SHM_CHANNELS_CONFIG *channel_config_pointer = (SHM_CHANNELS_CONFIG *)get_channel_config_pointer(channel_index);
    if (channel_config_pointer == NULL)
    {
        printf("error \n");
        return -1;
    }
    else
    {
        int queue_index = get_send_queue_index(channel_config_pointer, src_addr);
        //获得tail相对于queue_begin_addr 的偏移
        int tail_offset = channel_config_pointer->queue_tail[queue_index];
        int queue_size = channel_config_pointer->queue_channel_size[queue_index];
        int remain_size = queue_size - tail_offset;
        //printf("tail_offset : %d , queue_size : %d remain_size : %d \n",tail_offset,queue_size,remain_size);
        return remain_size;
    }
}

//获得接收队列的剩余空间
static int get_remain_recv_size(int channel_index, string src_addr)
{
    SHM_CHANNELS_CONFIG *channel_config_pointer = (SHM_CHANNELS_CONFIG *)get_channel_config_pointer(channel_index);
    if (channel_config_pointer == NULL)
    {
        printf("error \n");
        return -1;
    }
    else
    {
        int queue_index = get_recv_queue_index(channel_config_pointer, src_addr);
        //获得head 相对于queue_begin_addr的偏移
        int head_offset = channel_config_pointer->queue_head[queue_index];
        int queue_size = channel_config_pointer->queue_channel_size[queue_index];
        int remain_size = queue_size - head_offset;
        //printf("head_offset : %d , queue_size : %d remain_size : %d \n",head_offset,queue_size,remain_size);
        return remain_size;
    }
}

static int get_channel_send_queue_size(int channel_index, string src_addr)
{
    SHM_CHANNELS_CONFIG *channel_config_pointer = (SHM_CHANNELS_CONFIG *)get_channel_config_pointer(channel_index);
    if (channel_config_pointer == NULL)
    {
        printf("error \n");
        return 0;
    }
    else
    {
        int queue_index = get_send_queue_index(channel_config_pointer, src_addr);
        int queue_size = channel_config_pointer->queue_channel_size[queue_index];
        return queue_size;
    }
}

//获得发送队列的 起始指针
static void *get_send_queue_begin_pointer(int channel_index, string src_addr)
{
    SHM_CHANNELS_CONFIG *channel_config_pointer = (SHM_CHANNELS_CONFIG *)get_channel_config_pointer(channel_index);
    if (channel_config_pointer == NULL)
    {
        printf("error \n");
        return NULL;
    }
    else
    {
        int queue_index = get_send_queue_index(channel_config_pointer, src_addr);
        int begin_addr = channel_config_pointer->queue_begin_addr[queue_index];
        return static_cast<void *>(((char *)shm_head) + begin_addr);
    }
}

//获得接收队列的 起始指针
static void *get_recv_queue_begin_pointer(int channel_index, string src_addr)
{
    SHM_CHANNELS_CONFIG *channel_config_pointer = (SHM_CHANNELS_CONFIG *)get_channel_config_pointer(channel_index);
    if (channel_config_pointer == NULL)
    {
        printf("error \n");
        return NULL;
    }
    else
    {
        int queue_index = get_recv_queue_index(channel_config_pointer, src_addr);
        int begin_addr = channel_config_pointer->queue_begin_addr[queue_index];
        return static_cast<void *>(((char *)shm_head) + begin_addr);
    }
}

//获得接收队列的head的偏移
static int get_channel_recv_queue_head(int channel_index, string src_addr)
{
    SHM_CHANNELS_CONFIG *channel_config_pointer = (SHM_CHANNELS_CONFIG *)get_channel_config_pointer(channel_index);
    if (channel_config_pointer == NULL)
    {
        printf("error \n");
        return 0;
    }
    else
    {
        int queue_index = get_recv_queue_index(channel_config_pointer, src_addr);
        int offset = channel_config_pointer->queue_head[queue_index];
        return offset;
    }
}

//获得接收队列的 head指针
static void *get_channel_recv_queue_head_pointer(int channel_index, string src_addr)
{
    SHM_CHANNELS_CONFIG *channel_config_pointer = (SHM_CHANNELS_CONFIG *)get_channel_config_pointer(channel_index);
    if (channel_config_pointer == NULL)
    {
        printf("error \n");
        return NULL;
    }
    else
    {
        int queue_index = get_recv_queue_index(channel_config_pointer, src_addr);
        int offset = channel_config_pointer->queue_begin_addr[queue_index] + channel_config_pointer->queue_head[queue_index];
        void *recv_head_pointer = static_cast<void *>((char *)shm_head + offset);
        //printf("recv queue head offset : %d and %p \n",offset,recv_head_pointer);
        return recv_head_pointer;
    }
}

static int get_recv_msg_num(int channel_index, string src_addr)
{
    SHM_CHANNELS_CONFIG *channel_config_pointer = (SHM_CHANNELS_CONFIG *)get_channel_config_pointer(channel_index);
    if (channel_config_pointer == NULL)
    {
        printf("error \n");
        return -1;
    }
    else
    {
        int queue_index = get_recv_queue_index(channel_config_pointer, src_addr);
        return channel_config_pointer->msg_num[queue_index];
    }
}

//发送消息的时候 add_msg
static int add_send_msg_num(int channel_index, string src_addr)
{
    SHM_CHANNELS_CONFIG *channel_config_pointer = (SHM_CHANNELS_CONFIG *)get_channel_config_pointer(channel_index);
    if (channel_config_pointer == NULL)
    {
        printf("error \n");
        return -1;
    }
    else
    {
        int queue_index = get_send_queue_index(channel_config_pointer, src_addr);
        channel_config_pointer->msg_num[queue_index]++;
        return 0;
    }
}

//接收消息的时候 sub_msg
static int sub_recv_msg_num(int channel_index, string src_addr)
{
    SHM_CHANNELS_CONFIG *channel_config_pointer = (SHM_CHANNELS_CONFIG *)get_channel_config_pointer(channel_index);
    if (channel_config_pointer == NULL)
    {
        printf("error \n");
        return -1;
    }
    else
    {
        int queue_index = get_recv_queue_index(channel_config_pointer, src_addr);
        if (channel_config_pointer->msg_num[queue_index] > 0)
        {
            channel_config_pointer->msg_num[queue_index]--;
            return 0;
        }
        else
        {
            printf("error channel_config_pointer->msg_num < 0 \n");
            return -1;
        }
    }
}

//获得发送队列的tail的偏移
static int get_channel_send_queue_tail(int channel_index, string src_addr)
{
    SHM_CHANNELS_CONFIG *channel_config_pointer = (SHM_CHANNELS_CONFIG *)get_channel_config_pointer(channel_index);
    if (channel_config_pointer == NULL)
    {
        printf("error \n");
        return -1;
    }
    else
    {
        int queue_index = get_send_queue_index(channel_config_pointer, src_addr);
        int offset = channel_config_pointer->queue_tail[queue_index];
        return offset;
    }
}

//获得发送队列的 tail指针
static char *get_channel_send_queue_tail_pointer(int channel_index, string src_addr)
{
    SHM_CHANNELS_CONFIG *channel_config_pointer = (SHM_CHANNELS_CONFIG *)get_channel_config_pointer(channel_index);
    if (channel_config_pointer == NULL)
    {
        printf("error \n");
        return NULL;
    }
    else
    {
        int queue_index = get_send_queue_index(channel_config_pointer, src_addr);
        int offset = channel_config_pointer->queue_begin_addr[queue_index] + channel_config_pointer->queue_tail[queue_index];
        char *send_tail_pointer = (char *)shm_head + offset;
        //printf("send queue tail offset : %d and %p \n",offset,send_tail_pointer);
        return send_tail_pointer;
    }
}

//修改发送队列的tail指针
static int add_channel_send_queue_tail_offset(int channel_index, int offset, string src_addr)
{
    SHM_CHANNELS_CONFIG *channel_config_pointer = (SHM_CHANNELS_CONFIG *)get_channel_config_pointer(channel_index);
    if (channel_config_pointer == NULL)
    {
        printf("error \n");
        return -1;
    }
    else
    {
        int queue_index = get_send_queue_index(channel_config_pointer, src_addr);
        channel_config_pointer->queue_tail[queue_index] = (channel_config_pointer->queue_tail[queue_index] + offset) % channel_config_pointer->queue_channel_size[queue_index];
        //printf("set send queue tail offset : %d \n",channel_config_pointer->queue_begin_addr[queue_index] +  channel_config_pointer->queue_tail[queue_index] );
        return 0;
    }
}

//修改接收队列的head指针
static int add_channel_recv_queue_head_offset(int channel_index, int offset, string src_addr)
{
    SHM_CHANNELS_CONFIG *channel_config_pointer = (SHM_CHANNELS_CONFIG *)get_channel_config_pointer(channel_index);
    if (channel_config_pointer == NULL)
    {
        printf("error \n");
        return -1;
    }
    else
    {
        int queue_index = get_recv_queue_index(channel_config_pointer, src_addr);
        channel_config_pointer->queue_head[queue_index] = (channel_config_pointer->queue_head[queue_index] + offset) % channel_config_pointer->queue_channel_size[queue_index];
        //printf("set recv queue head offset : %d \n",channel_config_pointer->queue_begin_addr[queue_index] +  channel_config_pointer->queue_head[queue_index]);
        return 0;
    }
}

static int my_strncpy(char *des, char *sour, int len)
{
    MSG msg;
    msg.msg_len = len;
    memcpy(msg.msg, sour, len);
    //拷贝
    memcpy(des, &msg, len + sizeof(int));
    return 0;
}

static int my_strncpy_two_segment_copy(char *des, char *sour, int msg_len, int total_len, int remain_send_size, void *send_queue_begin_addr)
{
    MSG msg;
    msg.msg_len = msg_len;
    memcpy(msg.msg, sour, msg_len);
    //拷贝一部分
    void *source = (void *)&msg;
    memcpy(des, source, remain_send_size);
    //拷贝剩下的一部分
    memcpy(send_queue_begin_addr, static_cast<void *>((char *)source + remain_send_size), total_len - remain_send_size);
    return 0;
}

static int handle_tbus_recv(string src_addr, string dest_addr, char *msg, int &msg_len)
{
    //获得接收队列的head指针
    int channel_index = get_channel_index(src_addr, dest_addr);
    int msg_num = get_recv_msg_num(channel_index, src_addr);
    if (msg_num <= 0)
    {
        //printf("msg num <=0 \n");
        return -1;
    }
    void *send_head_pointer = get_channel_recv_queue_head_pointer(channel_index, src_addr);
    //获得剩余的空间
    int remain_recv_size = get_remain_recv_size(channel_index, src_addr);
    //获得接收队列的起始地址
    void *recv_queue_begin_addr = get_recv_queue_begin_pointer(channel_index, src_addr);

    //判断空间是否够读
    if (remain_recv_size < static_cast<int>(sizeof(int)))
    //剩余的空间小于int
    {
        //获得长度
        int *p_int = new int;
        void *dest = (void *)p_int;
        void *src = (void *)send_head_pointer;
        memcpy(dest, src, remain_recv_size);
        memcpy(static_cast<void *>((char *)dest + remain_recv_size), recv_queue_begin_addr, sizeof(int) - remain_recv_size);
        msg_len = *p_int;
        //拷贝内容
        src = static_cast<void *>((char *)recv_queue_begin_addr + sizeof(int) - remain_recv_size);
        memcpy(msg, src, msg_len);
    }
    else
    //剩余的空间大于int
    {
        //获得长度
        MSG *msg_pointer = (MSG *)send_head_pointer;
        msg_len = msg_pointer->msg_len;
        if (msg_len > static_cast<int>(remain_recv_size - sizeof(int)))
        {
            //如果后面的空间小于msg的长度，那么就要分成两个部分读
            void *dest = (void *)msg;
            void *src = static_cast<void *>(((char *)send_head_pointer) + sizeof(int));
            int memcpy_len1 = remain_recv_size - sizeof(int);
            memcpy(dest, src, memcpy_len1);
            memcpy(static_cast<void *>((char *)dest + memcpy_len1), recv_queue_begin_addr, msg_len - memcpy_len1);
        }
        else
        {
            //如果后面的空间小于msg的长度，那么可以直接读取
            MSG *msg_pointer = (MSG *)send_head_pointer;
            memcpy(msg, msg_pointer->msg, msg_len);
        }
    }
    //msg_num --
    sub_recv_msg_num(channel_index, src_addr);
    //移动head指针
    add_channel_recv_queue_head_offset(channel_index, msg_len + sizeof(int), src_addr);
    return 0;
}

//接收所有地址
static int my_tbus_recv_all(string local_addr, string &peer_addr, char *msg, int &msg_len)
{
    int local_addr_int = inet_addr(local_addr.c_str());
    vector<string> channel_src_vector;
    int channels_num = shm_head->channels_num;
    for (int i = 0; i < channels_num; i++)
    {
        if (shm_head->pair_array[i].address1 == local_addr_int || shm_head->pair_array[i].address2 == local_addr_int)
        {
            if (shm_head->pair_array[i].address1 == local_addr_int)
            {
                string src_str(shm_head->pair_array[i].address2_str);
                channel_src_vector.push_back(src_str);
            }
            else
            {
                string src_str(shm_head->pair_array[i].address2_str);
                channel_src_vector.push_back(src_str);
            }
        }
    }

    for (unsigned int i = 0; i < channel_src_vector.size(); i++)
    {
        peer_addr = channel_src_vector[i];
        printf("peer addr [%s] \n", peer_addr.c_str());
        int ret = handle_tbus_recv(local_addr, peer_addr, msg, msg_len);
        if (ret == -1)
            continue;
        else
            return 0;
    }
    return -1;
}

//判断队列的空间是否足够
bool is_can_send(int msg_len, int channel_index, string src_addr)
{
    //获得tail的偏移
    int tail = get_channel_send_queue_tail(channel_index, src_addr);
    //获得head的偏移
    int head = get_channel_recv_queue_head(channel_index, src_addr);
    int send_queue_size = get_channel_send_queue_size(channel_index, src_addr);

    if (tail == head)
    {
        return true;
    }
    else if (tail > head)
    {
        int used = tail - head;
        int free_size = send_queue_size - used;
        if (free_size > msg_len + static_cast<int>(sizeof(int)))
        {
            return true;
        }
        return false;
    }
    else
    {
        //tail < head
        int free_size = (head - tail) - 1;
        if (free_size > msg_len + static_cast<int>(sizeof(int)))
        {
            return true;
        }
        return false;
    }
}

//通过tbus发送
int mytbus_send(string src_addr, string dest_addr, char *msg, int msg_len)
{
    //获得发送队列的tail的指针
    int channel_index = get_channel_index(src_addr, dest_addr);
    char *send_tail_pointer = get_channel_send_queue_tail_pointer(channel_index, src_addr);
    //获得剩余的空间
    int remain_send_size = get_remain_send_size(channel_index, src_addr);

    //判断队列的空间是否足够
    if (is_can_send(msg_len, channel_index, src_addr))
    {
        //判断是否需要分层两个部分写
        if (remain_send_size > (msg_len + static_cast<int>(sizeof(int))))
        {
            my_strncpy(send_tail_pointer, msg, msg_len);
        }
        else
        {
            //空间不够用，分层两段来写
            void *send_queue_begin_addr = get_send_queue_begin_pointer(channel_index, src_addr);
            my_strncpy_two_segment_copy(send_tail_pointer, msg, msg_len, msg_len + sizeof(int), remain_send_size, send_queue_begin_addr);
        }
        //移动tail指针
        add_channel_send_queue_tail_offset(channel_index, msg_len + sizeof(int), src_addr);
        //msg_num ++
        add_send_msg_num(channel_index, src_addr);
    }
    else
    {
        printf("tbus full\n");
    }
    return 0;
}

//通过tbus接收
//dest_addr == 0 表示监听所有的地址
//这个调用的行为和原来的tbus是有一点区别的
int mytbus_recv(string src_addr, string &dest_addr, char *msg, int &msg_len)
{
    if (dest_addr == "0")
    {
        //接收所有的地址
        return my_tbus_recv_all(src_addr, dest_addr, msg, msg_len);
    }
    else
    {
        return handle_tbus_recv(src_addr, dest_addr, msg, msg_len);
    }
}