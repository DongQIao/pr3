#include<stdlib.h>
#include<stdio.h>
#include "mem.h"
#include<sys/mman.h>
#include<fcntl.h>
#include<unistd.h>

typedef struct  head_t{
    int size;
    void* last;
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
/**void* ops(NODE* rt,int size)
{
    NODE* pt,*qt;
    HEAD* head;
    int n_size;
    void* ptr;
    if(rt==n_head){
         pt=n_head->next;
         n_size=n_head->size;
         head=(HEAD*)n_head;
         n_head=(NODE*)((char*)n_head+sizeof(HEAD)+size);
         n_head->size=n_size-(size+sizeof(HEAD));
         n_head->next=pt;
         head->size=size;
         ptr=(void*)head+sizeof(HEAD);
         head->last=ptr+size;
         //printf("%p\n%p\n",head,n_head);
         return ptr;
         }
        pt=n_head;
    while(pt->next!=rt){pt=pt->next;}
        head=(HEAD*)rt;
        n_size=rt->size;
        qt=rt->next;
        rt=(NODE*)((char*)rt+sizeof(HEAD)+size);
        rt->size=n_size-(size+sizeof(HEAD));
        rt->next=qt;
        pt->next=rt;
        head->size=size;
        ptr=(void*)((char*)head+sizeof(HEAD));
        head->last=ptr+size;
        return ptr;
}**/
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
            n_head+=n_size;
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
    ht->last=ptr+size;
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

//*
int mem_free(void* ptr)
{
  HEAD *pt;
  NODE *rt,*qt,*lt;
  int n_size;
  if(ptr==NULL)return -1;
  pt=ptr-sizeof(HEAD);
  rt=n_head;
  while(rt!=NULL){
     if((void*)rt==pt->last){
       n_size=pt->size+sizeof(HEAD);
       qt=(NODE*)pt;
       qt->size=n_size+rt->size;
       qt->next=rt->next;
       if(rt==n_head){ n_head=qt; return 0; }
       else{
         lt=n_head;
         while(lt->next!=rt){ lt=lt->next; }
         lt->next=qt;
       }
     }
     rt=rt->next;
     }
  rt=n_head;
  while(rt!=NULL){
     if((void*)rt+rt->size==(void*)pt){
       rt->size=rt->size+pt->size+sizeof(HEAD);
       return 0;
     }
     rt=rt->next;
     }
   rt=n_head;
   n_size=pt->size;
   qt=(NODE*)pt;
   while((rt<qt)&&((rt->next)>qt)){rt=rt->next;}
      qt->size=n_size+sizeof(HEAD);
      qt->next=rt->next;
      rt->next=qt;
      return 0;
}
//*/
/*
int mem_free(void* ptr)
{
    HEAD *ht;
    NODE *pt,*qt,*rt;
    int n_size;
    int flag=0;
    if(ptr==NULL){
        m_error=E_BAD_POINTER;
        return-1;
    }
    ht=(HEAD*)(ptr-sizeof(HEAD));
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
*/
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
    void* ptr,*ptr1;
    int i;
    if(mem_init(10000)==-1){
            perror("mem_init");
            exit(1);
            }
    for(i=0;i<3;i++){
            printf("%d\t%d\n",(int)sizeof(HEAD),(int)sizeof(NODE));
            ptr=mem_alloc(32,M_BESTFIT);
            printf("%p\n",ptr);
            ptr1=mem_alloc(64,M_BESTFIT);
            printf("%p\n",ptr1);
            printf("%d\n",n_head->size);
            ptr=mem_alloc(16,M_BESTFIT);
            printf("%p\n",ptr);
            mem_free(ptr1);
            }
    printf("\n\n");
    dump();
    return 0;
}
