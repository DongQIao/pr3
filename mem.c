#include<stdlib.h>
#include<stdio.h>
#include "mem.h"
#include<sys/mman.h>
#include<fcntl.h>
#include<unistd.h>

typedef struct  head_t{
    int size;
    int magic;
    }HEAD;

typedef struct node_t{
    int size;
    struct node_t* next;
    }NODE;

NODE* n_head;
int m_error;

int mem_init(int size_of_region)
{
    int fd=open("/dev/zero",O_RDWR);
    void *ptr=mmap(NULL,size_of_region,PROT_READ|PROT_WRITE,MAP_PRIVATE,fd,0);
    if(ptr==NULL){
        perror("mmap");
        return -1;
    }
    close(fd);
    n_head=ptr;
    n_head->size=size_of_region;
    n_head->next=NULL;
    //printf("%p\n",n_head);
    return 0;
}
void* m_play(NODE* rt,int size)
{
    NODE *pt,*qt;
    HEAD *ht;
    void* ptr;
    int n_size,t_size;
    n_size=size+sizeof(HEAD);
    if(rt==n_head){
        if(n_head->size==n_size){
            n_head=n_head->next;
        }
        else{
            t_size=n_head->size-n_size;
            n_head=(NODE*)((void*)n_head+n_size);
            n_head->size=t_size;
            n_head->next=rt->next;
        }
    }
    else{
        pt=n_head;
        while(pt->next!=rt){
            pt=pt->next;
        }
        if(rt->size==n_size){
            pt->next=rt->next;
        }
        else{
            qt=(NODE*)((void*)rt+n_size);
            qt->size=rt->size-n_size;
            qt->next=rt->next;
            pt->next=qt->next;
        }
    }
    ht=(HEAD*)rt;
    ht->size=size;
    ptr=(void*)ht+sizeof(HEAD);
    ht->magic=1234;
    return ptr;
}

void* mem_alloc(int size,int style)
{
    NODE *pt,*rt;
    void* ptr;
    int n_size;
    int flag=0;
    if(n_head==NULL){
        m_error=E_NO_SPACE;
        return NULL;
    }
    if(size%8!=0){
        size=(size/8+1)*8;
    }
    n_size=size+sizeof(HEAD);
    pt=n_head;
    rt=pt;
    if(style==M_BESTFIT){
        while(pt!=NULL){
            if((pt->size-n_size)>=sizeof(NODE)||pt->size==n_size){
                    flag=1;
                    if(pt->size<rt->size){
                        rt=pt;
                    }
            }
            pt=pt->next;
        }
        if(flag==0){
            m_error=E_NO_SPACE;
            return NULL;
        }
        ptr=m_play(rt,size);
        return ptr;
    }
    if(style==M_WORSTFIT){
        while(pt!=NULL){
            if(pt->size>rt->size){
                rt=pt;
            }
            pt=pt->next;
        }
        if(rt->size-n_size>=sizeof(NODE)||rt->size==n_size){
            flag=1;
        }
        if(flag==0){
            m_error=E_NO_SPACE;
            return NULL;
        }
        ptr=m_play(rt,size);
        return ptr;
    }
    if(style==M_FIRSTFIT){
            while(pt!=NULL){
                if(pt->size-n_size>=sizeof(NODE)||pt->size==n_size){
                    rt=pt;
                    flag=1;
                    break;
                }
            }
            if(flag==0){
                m_error=E_NO_SPACE;
                return NULL;
            }
            ptr=m_play(rt,size);
            return ptr;
    }
    m_error=E_BAD_ARGS;
    return NULL;
}

int mem_free(void* ptr)
{
    HEAD *ht;
    NODE *pt,*qt,*rt;
    int n_size;
    int flag=0;
    ht=(HEAD*)(ptr-sizeof(HEAD));
    if(ht->magic!=1234){
        m_error=E_BAD_POINTER;
        return-1;
    }
    ht->magic=4321;
    pt=(NODE*)ht;
    n_size=ht->size+sizeof(HEAD);
    pt->size=n_size;
    pt->next=NULL;
    qt=n_head;
    if(n_head==NULL){
        n_head=pt;
        return 0;
    }
    while(qt->next!=NULL){
        qt=qt->next;
    }
    qt->next=pt;
    rt=n_head;
    while(rt!=NULL){
        if((void*)rt+rt->size==(void*)pt){
            flag=1;
            break;
        }
        rt=rt->next;
    }
    if(flag==1){
        rt->size+=pt->size;
        qt->next=pt->next;
        flag=0;
    }
    else{
        rt=pt;
    }
    qt=n_head;
    pt=qt;
    while(qt!=NULL){
        if((void*)rt+rt->size==(void*)qt){
            flag=1;
            break;
        }
        pt=qt;
        qt=pt->next;
    }
    if(flag==1){
        if(qt==n_head){
            n_head=qt->next;
            rt->size+=qt->size;

            return 0;
        }
        pt->next=qt->next;
        rt->size+=qt->size;
    }
    return 0;
}
void dump()
{
    NODE*pt;
    pt=n_head;
    while(pt!=NULL){
        printf("%p\t%d\n",pt,pt->size);
        pt=pt->next;
    }
}
int main()
{
    void* ptr1,*ptr2,*ptr3;
    int i;
    if(mem_init(10000)==-1){
            perror("mem_init");
            exit(1);
            }
    for(i=0;i<3;i++){
            printf("NO\t%d\n",i);
            ptr1=mem_alloc(32,M_WORSTFIT);
            printf("%p\n",ptr1);
            ptr2=mem_alloc(64,M_FIRSTFIT);
            printf("%p\n",ptr2);
            ptr3=mem_alloc(16,M_BESTFIT);
            printf("%p\n",ptr3);
            dump();
            mem_free(ptr1);
            mem_free(ptr1);
            mem_free(ptr3);
            mem_free(ptr2);
            dump();
            }
    return 0;
}
