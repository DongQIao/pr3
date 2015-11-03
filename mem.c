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
    int fd=open("/dev/zero", O_RDWR);
    void *ptr=mmap(NULL, size_of_region, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (ptr == MAP_FAILED) { 
      perror("mmap"); 
      return -1; 
      }
      close(fd);
      n_head=ptr;
      n_head->size=size_of_region;
      n_head->next=NULL;
      //printf("%d\n",n_head->size);
      printf("%p\n",ptr);
    return 0;
}

void* ops(NODE* rt,int size)
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
}
void* mem_alloc(int size,int style)
{
   NODE *pt,*qt,*rt;
   HEAD* head;
   void* ptr;
   int t_size,sub;
   if(size%8!=0){size=(size/8)*8+8;}
   if(style==M_BESTFIT){//style = best
      pt=n_head;
      rt=NULL;
      t_size=size+sizeof(HEAD);
      sub=9999999;
      while(pt!=NULL){
        if(((pt->size)-t_size>=(int)sizeof(NODE))&&((pt->size)-t_size<sub)){
          //printf("%d\t%d\t%d\n",sub,t_size,pt->size);
          sub=(pt->size)-t_size;
          rt=pt;
        }
        pt=pt->next;
      }
      if(rt==NULL){
        m_error=E_NO_SPACE;
        return NULL;
       }
       ptr=ops(rt,size);
       return ptr;
      }
   if(style==M_WORSTFIT){  //style = worst
   pt=n_head;
   t_size=0;
   rt=NULL;
   while(pt!=NULL){
       if((pt->size)>t_size){
          t_size=pt->size;
          rt=pt;
          }
          pt=pt->next;
          }
       if(t_size<size+sizeof(HEAD)+sizeof(NODE)){
        m_error=E_NO_SPACE;
        return NULL;
       }
        ptr=ops(rt,size);
        return ptr;
   }
   if(style==M_FIRSTFIT){// style =first
   pt=n_head;
   rt=NULL;
   t_size=size+sizeof(HEAD);
   while(pt!=NULL){
     if((pt->size)-sizeof(NODE)>=t_size){
          rt=pt;
          break;
          }
      pt=pt->next;
   }
   if(rt==NULL){
        m_error=E_NO_SPACE;
        return NULL;
       }
       ptr=ops(rt,size);
       return ptr;
   }
   m_error=99;                                        //no this style
   return NULL;
}

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
void dump()
{
 NODE* pt;
 pt=n_head;
 while(pt!=NULL){
 printf("%p\t%d\n",pt,pt->size);
 pt=pt->next;
 }
}
int main()
{
  void* ptr,*ptr1;
  int d=sizeof(HEAD);
  int i;
  if(mem_init(10000)==-1){
  perror("mem_init");
  exit(1);
  }
  for(i=0;i<3;i++){
  printf("%d\t%d\n",(int)sizeof(HEAD),(int)sizeof(NODE));
  ptr=mem_alloc(16,M_BESTFIT);
  printf("%p\n",ptr);
  ptr1=mem_alloc(16,M_BESTFIT);
  printf("%p\n",ptr1);
  printf("%d\n",n_head->size);
  ptr=mem_alloc(16,M_BESTFIT);
  printf("%p\n",ptr);
  mem_free(ptr1);
  }
  printf("\n\n\n");
  dump();
  return 0;
}
