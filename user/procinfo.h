#ifndef _PROCINFO_H_
#define _PROCINFO_H_

enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

struct procinfo {
  char name[16];               // Имя процесса
  int pid;                     // ID процесса
  int ppid;                    // ID родительского процесса
  enum procstate state;        // Состояние процесса
};

#endif

