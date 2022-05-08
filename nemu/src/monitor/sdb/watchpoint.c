#include "common.h"
#include "sdb.h"

#define NR_WP 32


static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].value=-1;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

WP* new_wp(){
  if(free_->NO==NR_WP-1) assert(0);
  WP *tmp = free_;
  free_=tmp->next;
  tmp->next=head;
  head=tmp;
  return head;
}

void free_wp(WP* wp){
  assert(head != NULL);
  WP* tmp = NULL;
  
  if(wp!=head){
    for(WP *p=head;p->next!=NULL;p=p->next){
      if(p->next==wp){
        tmp=p->next;
        p->next=p->next->next;
        break;
      } 
    }
  }else{
    tmp=head;
    head=head->next;
  }
  assert(tmp!=NULL);
  tmp->next=free_;
  free_=tmp;
}

WP* find_wp(int no){
  
  for(WP *p=head;p!=NULL;p=p->next){
    if(p->NO==no)
      return p;
  }
  assert(0);
  return NULL;
}

WP** check_diff(int* total_diff){
  static WP* diff[NR_WP]={};
  static bool success;
  *total_diff = 0;
  for(WP *wp=head;wp!=NULL;wp=wp->next){
    word_t val = expr(wp->expression,&success);
    if(val!=wp->value){
      wp->value = val;
      diff[*total_diff++] = wp;
    }
  }
  return diff;
}


void print_watchlist(){

  printf("Num\tWhat\tValue\t\n");
  static bool success;
  for(WP *wp = head;wp!=NULL;wp=wp->next){
    word_t val = expr(wp->expression,&success);
    wp->value = val;
    printf("%d\t%s\t0x%016lx\n",wp->NO,wp->expression,val);
  }

}

void print_watchpoint(WP *wp){
  }

