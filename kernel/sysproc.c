#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "date.h"

uint64
sys_exit(void)
{
  int n;
  int errCheck = argint(0, &n);
  if (errCheck < 0) return -1;
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
  int errCheck = argaddr(0, &p);
  if (errCheck < 0) return -1;
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
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
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

  int errCheck = argint(0, &pid);
  if (errCheck < 0) return -1;
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

// return the input mask value for strace sys_call
uint64
sys_trace(void)
{
  int mask;
  int flag = argint(0, &mask);
  if (flag < 0)
    return -1;
  myproc()->mask = mask;
  return 0;
}

uint64
sys_sigreturn(void)
{
  restore();
  myproc() -> is_sigalarm = 0;
  return myproc() -> trapframe -> a0;
}

void
restore()
{
  struct proc * p = myproc();
  p -> dup_trapframe -> kernel_satp = p -> trapframe -> kernel_satp;
  p -> dup_trapframe -> kernel_hartid = p -> trapframe -> kernel_hartid;
  p -> dup_trapframe -> kernel_sp = p -> trapframe -> kernel_sp;
  p -> dup_trapframe -> kernel_trap = p -> trapframe -> kernel_trap;
  * (p -> trapframe) = * (p -> dup_trapframe);
  return;
}

// waitx syscall
uint64
sys_waitx(void)
{
  uint64 addr, addr1, addr2;
  uint wtime, rtime;
  if (argaddr(0, &addr) < 0)
    return -1;
  if (argaddr(1, &addr1) < 0) // user virtual memory
    return -1;
  if (argaddr(2, &addr2) < 0)
    return -1;
  int ret = waitx(addr, &wtime, &rtime);
  struct proc *p = myproc();
  if (copyout(p->pagetable, addr1, (char *)&wtime, sizeof(int)) < 0)
    return -1;
  if (copyout(p->pagetable, addr2, (char *)&rtime, sizeof(int)) < 0)
    return -1;
  return ret;
}

uint64
sys_set_priority(void)
{
  int new_static_priority;
  int proc_pid;
  int flg1 = argint(0, &new_static_priority);
  if (flg1 < 0)
    return -1;
  int flg2 = argint(1, &proc_pid);
  if (flg2 < 0)
    return -1;
  return set_priority(new_static_priority, proc_pid);
}

uint64
sys_settickets(void)
{
  ;
}