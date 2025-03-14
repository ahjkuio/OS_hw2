#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

extern struct proc proc[NPROC];
extern struct spinlock wait_lock;

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


uint64 sys_ps_listinfo(void) {
    struct proc *p;
    uint64 plist_addr;
    int lim;
    int written = 0;

    // Получаем аргументы
    argaddr(0, &plist_addr);
    argint(1, &lim);

    if (plist_addr == 0) {
        int count = 0;
        for (p = proc; p < &proc[NPROC]; p++) {
            acquire(&p->lock);
            if (p->state != UNUSED)
                count++;
            release(&p->lock);
        }
        return count;
    }
  
    for (p = proc; p < &proc[NPROC]; p++) {
        acquire(&p->lock);
      
        if (p->state != UNUSED) {
            // Проверяем лимит перед копированием 
            if (written >= lim) {
                release(&p->lock);
                return -2; // Недостаточный размер буфера
            }

            // Заполняем структуру напрямую
            struct procinfo pi;
            safestrcpy(pi.name, p->name, sizeof(pi.name));
            pi.pid = p->pid;
            pi.state = p->state;
            
            // Получаем ppid - только здесь нужен wait_lock
            acquire(&wait_lock);
            if (p->parent) {
                // Захватываем лок родительского процесса перед доступом к нему
                acquire(&p->parent->lock);
                pi.ppid = p->parent->pid;
                release(&p->parent->lock);
            } else {
                pi.ppid = 0;
            }
            release(&wait_lock);

            // Пытаемся скопировать в пользовательский буфер
            if (copyout(myproc()->pagetable, 
                        plist_addr + written * sizeof(struct procinfo), 
                        (char*)&pi, sizeof(pi)) < 0) {
                release(&p->lock);
                return -3; // Ошибка копирования в пользовательское пространство
            }

            written++;
        }
      
        release(&p->lock);
    }

    return written; // Возвращаем количество записанных процессов
}
