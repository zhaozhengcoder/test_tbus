#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <libxml/xpath.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlstring.h>
#include <vector>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "../include/shm_head.h"
using namespace std;

struct CHANNELS
{
    int Priority;
    string Address1;
    string Address2;
    int SendSize;
    int RecvSize;
    string Desc;
};

//解析每一个channels对应的数据
CHANNELS parseChannels(xmlDocPtr doc, xmlNodePtr cur)
{
	xmlChar* key;
	cur=cur->xmlChildrenNode;
    CHANNELS item_channel;
	while(cur != NULL){
		if(!xmlStrcmp(cur->name, (const xmlChar *)"Priority")){
            key = xmlNodeGetContent(cur);
            
            item_channel.Priority = atoi( (char *) (key));
			printf("Priority: %s\n", key);
			xmlFree(key);
            key =NULL;
		}
        if(!xmlStrcmp(cur->name, (const xmlChar *)"Address")){
            key = xmlNodeGetContent(cur);
			printf("Address1: %s\n", key);
            string str_address1 = (char *)key;
            item_channel.Address1 = str_address1;
            xmlFree(key);
            key =NULL;
            cur=cur->next;
            cur=cur->next;
            if(!xmlStrcmp(cur->name, (const xmlChar *)"Address")){
                key = xmlNodeGetContent(cur);
                
                string str_address2 =(char *)key;
                item_channel.Address2 = str_address2;
            }
			xmlFree(key);
            key =NULL;
		}
        if(!xmlStrcmp(cur->name, (const xmlChar *)"SendSize")){
            key = xmlNodeGetContent(cur);
            item_channel.SendSize = atoi( (char *) (key));
			printf("SendSize: %s\n", key);
			xmlFree(key);
            key =NULL;
		}       
        if(!xmlStrcmp(cur->name, (const xmlChar *)"RecvSize")){
            key = xmlNodeGetContent(cur);
            item_channel.RecvSize =atoi( (char *) (key));
			printf("RecvSize: %s\n", key);
			xmlFree(key);
            key =NULL;
		}   
        if(!xmlStrcmp(cur->name, (const xmlChar *)"Desc")){
            key = xmlNodeGetContent(cur);
            string str_desc = (char *)key;
            item_channel.Desc = str_desc;
			printf("Desc: %s\n", key);
			xmlFree(key);
            key =NULL;
		}   
		cur=cur->next; 
	}
	return item_channel;
}

//解析xml
vector<CHANNELS> parseDoc(char *docname, int & GCIM_SHM_KEY){
	vector<CHANNELS> channels_res;
	xmlDocPtr doc;
	xmlNodePtr cur;
	doc = xmlParseFile(docname);
	if(doc == NULL)
	{
		fprintf(stderr, "Document not parse successfully. \n");
		return channels_res;
	}
	cur = xmlDocGetRootElement(doc);
	if(cur == NULL){
		fprintf(stderr, "empty document\n");
		xmlFreeDoc(doc);
		return channels_res;
	}
	if(xmlStrcmp(cur->name, (const xmlChar *)"TbusGCIM"))
    {
		fprintf(stderr, "document of the wrong type, root node != TbusGCIM");
		xmlFreeDoc(doc);
		return channels_res;
	}
	cur = cur->xmlChildrenNode;
	while(cur != NULL)
	{
        //printf("cur name  %s \n",cur->name);
        if(!xmlStrcmp(cur->name, (const xmlChar *)"GCIMShmKey")){
            xmlChar* key = xmlNodeGetContent(cur);
            GCIM_SHM_KEY = atoi( (char * )key);
        }
		if(!xmlStrcmp(cur->name, (const xmlChar *)"Channels")){
			channels_res.push_back(parseChannels(doc, cur)); 
		}
		cur = cur->next;
	}
	xmlFreeDoc(doc); 
	return channels_res;
}

int get_mem_size(const vector<CHANNELS> & channels_res)
{
    int size=0;
    //头部的大小
    size = size + sizeof(SHM_HEAD);
    //每个channel控制信息的大小
    size = size + sizeof(SHM_CHANNELS_CONFIG) * channels_res.size();
    for(int i=0; i<channels_res.size(); i++)
    {
        size = size + channels_res[i].SendSize + channels_res[i].RecvSize;
    }
    return size;
}

int init_shm(int key,const vector<CHANNELS> & channels_res)
{
    int mem_size = get_mem_size(channels_res);
    key = key<<2;
    int shm_id = shmget(key,mem_size, 0666|IPC_CREAT);
    if(shm_id==-1)
    {
        printf("shmget error \n");
        return -1;
    }

    SHM_HEAD * shm_head = (SHM_HEAD *)shmat(shm_id,NULL,0);
    shm_head->channels_num = channels_res.size();

    //init head
    for(int i=0;i<channels_res.size();i++)
    {
        if(i==0)
        {
            shm_head->channels_config_begin_addr[i] = sizeof(SHM_HEAD);
            shm_head->channels_config_end_addr[i]   = sizeof(SHM_HEAD) + sizeof(SHM_CHANNELS_CONFIG);
        }
        else
        {
            shm_head->channels_config_begin_addr[i] = shm_head->channels_config_end_addr[i-1];
            shm_head->channels_config_end_addr[i]   = shm_head->channels_config_begin_addr[i] + sizeof(SHM_CHANNELS_CONFIG);
        }
        strncpy(shm_head->pair_array[i].address1_str, (channels_res[i].Address1).c_str(), ADDRESS_STR_LEN);
        strncpy(shm_head->pair_array[i].address2_str, (channels_res[i].Address2).c_str(), ADDRESS_STR_LEN);
        shm_head->pair_array[i].address1 = min(inet_addr((channels_res[i].Address1).c_str()),inet_addr((channels_res[i].Address2).c_str()));
        shm_head->pair_array[i].address2 = max(inet_addr((channels_res[i].Address1).c_str()),inet_addr((channels_res[i].Address2).c_str()));
        shm_head->pair_array[i].channel_index = i;
    } 

    //init config
    for(int i=0;i<channels_res.size();i++)
    {
        SHM_CHANNELS_CONFIG * cur_channel_config = ((SHM_CHANNELS_CONFIG * )(((void * )shm_head) + shm_head->channels_config_begin_addr[i]));
        cur_channel_config->pair.address     = inet_addr((channels_res[i].Address1).c_str());
        cur_channel_config->pair.queue_index = 0;
        cur_channel_config->queue_begin_addr[0] = sizeof(SHM_HEAD) + sizeof(SHM_CHANNELS_CONFIG) * channels_res.size();
        cur_channel_config->queue_end_addr[0]   = cur_channel_config->queue_begin_addr[0] + channels_res[i].SendSize;
        cur_channel_config->queue_begin_addr[1] = cur_channel_config->queue_end_addr[0];
        cur_channel_config->queue_end_addr[1]   = cur_channel_config->queue_begin_addr[1] + channels_res[i].RecvSize;  
        cur_channel_config->queue_channel_size[0] = channels_res[i].SendSize;
        cur_channel_config->queue_channel_size[1] = channels_res[i].RecvSize;
        cur_channel_config->msg_num[0] = 0;
        cur_channel_config->msg_num[1] = 0;
        cur_channel_config->queue_head[0] = 0; 
        cur_channel_config->queue_tail[0] = 0;
        cur_channel_config->queue_head[1] = 0;
        cur_channel_config->queue_tail[1] = 0;
    }
    return 0;
}

//获得发送队列的index
static int get_send_queue_index(SHM_CHANNELS_CONFIG * channel_config_pointer, int src_addr)
{
    //获得发送src作为发送地址的，对应的index
    if(channel_config_pointer != NULL)
    {
        int queue_index;
        if(channel_config_pointer->pair.address == src_addr)
            queue_index = 0;
        else
            queue_index = 1;
        return queue_index;
    }
    printf("channel_config_pointer == NULL \n");
    return -1;
}

//获得接收队列的index
static int get_recv_queue_index(const SHM_CHANNELS_CONFIG * channel_config_pointer, string src_addr)
{
    if(channel_config_pointer != NULL)
    {
        int queue_index;
        if(channel_config_pointer->pair.address == inet_addr(src_addr.c_str()))
            queue_index = 1;
        else
            queue_index = 0;
        return queue_index;
    }
    printf("channel_config_pointer == NULL \n");
    return -1;
}

int get_channels_info(int tbus_shmkey)
{
    tbus_shmkey = tbus_shmkey <<2;
    int shm_id = shmget(tbus_shmkey, 0, 0666|IPC_CREAT);
    if(shm_id < 0)
    {
        printf("shmget error \n");
        return -1;
    }
    SHM_HEAD * shm_head = (SHM_HEAD *)shmat(shm_id,NULL,0);

    printf("channels num is %d \n",shm_head->channels_num);

    int channels_num = shm_head->channels_num;
    for(int i=0; i<channels_num; i++)
    {
        printf("channel [%d]\n",i);
        SHM_CHANNELS_CONFIG * cur_channel_config = ((SHM_CHANNELS_CONFIG * )(((void * )shm_head) + shm_head->channels_config_begin_addr[i]));
        //send_index
        int index1 = get_send_queue_index(cur_channel_config, shm_head->pair_array[i].address1);
        //recv_index
        int index2 = get_send_queue_index(cur_channel_config, shm_head->pair_array[i].address2);

        int send_tail_offset = cur_channel_config->queue_tail[index1];
        int send_head_offset = cur_channel_config->queue_head[index1];

        int recv_tail_offset = cur_channel_config->queue_tail[index2];
        int recv_head_offset = cur_channel_config->queue_head[index2];

        printf("channel address : %s  --> address %s , channel size : %d \nhead offset : %d , tail offset: %d ,msg num : %d \n",
        shm_head->pair_array[i].address1_str, shm_head->pair_array[i].address2_str,
        cur_channel_config->queue_channel_size[index1], 
        send_tail_offset, send_head_offset, cur_channel_config->msg_num[index1]);

        printf("channel address : %s  <-- address %s , channel size : %d \nhead offset : %d , tail offset: %d ,msg num : %d \n",
        shm_head->pair_array[i].address1_str, shm_head->pair_array[i].address2_str,
        cur_channel_config->queue_channel_size[index2],
        recv_tail_offset, recv_head_offset,cur_channel_config->msg_num[index2]);
        printf("\n");
    }
}

int write_tbus(char * xml_file_name)
{
	int GCIM_SHM_KEY;
    //parse xml
    vector<CHANNELS> channels_res = parseDoc(xml_file_name,GCIM_SHM_KEY);
    //init shm
    init_shm(GCIM_SHM_KEY, channels_res);
    return 0;
}

int see_tbus(int GCIM_SHM_KEY)
{
    get_channels_info(GCIM_SHM_KEY);
    return 0;
}

int delete_tbus(int tbus_shmkey)
{
    tbus_shmkey = tbus_shmkey <<2;
    int shm_id = shmget(tbus_shmkey, 0, 0666|IPC_CREAT);
    if(shm_id < 0)
    {
        printf("shmget error \n");
        return -1;
    }
    int ret = shmctl(shm_id, IPC_RMID, NULL);
    if(ret!=0)
    {
        printf("shmctl ipc_rmid error \n");
        return -1;
    }
    printf("delete tbus\n");
    return 0;
}

int clean_tbus(int tbus_shmkey)
{
    tbus_shmkey = tbus_shmkey <<2;
    int shm_id = shmget(tbus_shmkey, 0, 0666|IPC_CREAT);
    if(shm_id < 0)
    {
        printf("shmget error \n");
        return -1;
    }
    SHM_HEAD * shm_head = (SHM_HEAD *)shmat(shm_id,NULL,0);

    //printf("channels num is %d \n",shm_head->channels_num);
    int channels_num = shm_head->channels_num;
    for(int i=0; i<channels_num; i++)
    {
        SHM_CHANNELS_CONFIG * cur_channel_config = ((SHM_CHANNELS_CONFIG * )(((void * )shm_head) + shm_head->channels_config_begin_addr[i]));
        cur_channel_config->queue_head[0] = 0;
        cur_channel_config->queue_head[1] = 0;
        cur_channel_config->queue_tail[0] = 0;
        cur_channel_config->queue_tail[1] = 0;
        cur_channel_config->msg_num[0] = 0;
        cur_channel_config->msg_num[1] = 0;
    }
    printf("clean tbus\n");
    return 0;
}

void usage(void)
{
   printf(
	"Eg : ./mytbus_mgr -W config.xml\n"
	"    -W  congfig.xml  create tbus by config.xml  \n"
    "    -S  GCIM_KEY     see-all tbus channel       \n"
    "    -X  GCIM_KEY     delete gcim                \n"
    "    -D  GCIM_KEY     clean tbus channel         \n"
	);
};

int main(int argc,char *argv[])
{
    if(argc==1)
    {
        usage();
        return -1;       
    }
    int ch;
    while((ch = getopt(argc, argv, "W:S:X:D:")) != -1) {
        switch(ch) {
            case 'W':
                //写tbus
                write_tbus(optarg);
                break;
            case 'S':
                //查看通道
                see_tbus(atoi(optarg));
                break;
            case 'X':
                //删除gcim
                delete_tbus(atoi(optarg));
                break;
            case 'D':
                //清空tbus
                //printf("d : %s \n",optarg);
                clean_tbus(atoi(optarg));
                break;
            default:
                usage();
                return -1;
        }
    }
	return 0;
}