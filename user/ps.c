#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/procinfo.h"

void print_state(enum procstate state) {
  switch(state) {
    case UNUSED: printf("unused"); break;
    case SLEEPING: printf("sleep"); break;
    case RUNNABLE: printf("runnable"); break;
    case RUNNING: printf("running"); break;
    case ZOMBIE: printf("zombie"); break;
    default: printf("unknown");
  }
}

int main(void) {
  // Сначала узнаем количество процессов
  int count = ps_listinfo(0, 0);
  if(count < 0) {
    fprintf(2, "ps: error getting process count\n");
    exit(1);
  }

  // Выделяем память под массив процессов
  struct procinfo *plist = malloc(count * sizeof(struct procinfo));
  if(!plist) {
    fprintf(2, "ps: malloc failed\n");
    exit(1);
  }

  // Получаем информацию о процессах
  int ret = ps_listinfo(plist, count);
  if(ret < 0) {
    fprintf(2, "ps: error getting process info\n");
    free(plist);
    exit(1);
  }

  // Выводим информацию в табличном виде
  printf("PID\tPPID\tSTATE\t\tNAME\n");
  for(int i = 0; i < ret; i++) {
    printf("%d\t%d\t", plist[i].pid, plist[i].ppid);
    print_state(plist[i].state);
    printf("\t%s\n", plist[i].name);
  }

  free(plist);
  exit(0);
}

