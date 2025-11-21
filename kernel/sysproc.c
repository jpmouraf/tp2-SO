#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include "pstat.h"

extern struct proc proc[NPROC];  
extern struct spinlock proc_lock;

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
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
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr)
      return -1;
    if(addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
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
  return kkill(pid);
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

extern int total_tickets; // Declarar a variável global se não estiver em um header
extern struct spinlock total_tickets_lock;

uint64
sys_settickets(void)
{
  int number;
  struct proc *p = myproc();

  if(argint(0, &number) < 0)
    return -1;

  if(number < 1){
    return -1;
  }

  // Atualizar o total de tickets global e do processo atual
  acquire(&total_tickets_lock); 
  
  // Se o processo for RUNNABLE ou RUNNING, subtraímos os bilhetes antigos antes de somar os novos.
  // Se for SLEEPING ou outra coisa, a atualização será feita quando ele se tornar RUNNABLE.
  if (p->state == RUNNABLE || p->state == RUNNING) {
      total_tickets -= p->tickets;
      total_tickets += number;
  }
  
  p->tickets = number;

  release(&total_tickets_lock);

  return 0; 
}

uint64
sys_getpinfo(void)
{
  uint64 user_pstat_addr;
  struct pstat ps;        
  struct proc *p;
  int i;

  // Obter o endereço do ponteiro de usuário
  if(argaddr(0, &user_pstat_addr) < 0)
    return -1;

  acquire(&proc_lock); 
  
  i = 0;
  for(p = proc; p < &proc[NPROC]; p++){
    
    if(p->state != UNUSED){
      ps.inuse[i] = 1;
      ps.pid[i] = p->pid;
      ps.tickets[i] = p->tickets;
      ps.ticks[i] = p->ticks; 
    } else {
      ps.inuse[i] = 0;
      ps.pid[i] = 0;
      ps.tickets[i] = 0;
      ps.ticks[i] = 0;
    }
    i++;
  }
  
  release(&proc_lock);

  // Copiar a estrutura preenchida do kernel para o espaço do usuário
  if(copyout(myproc()->pagetable, user_pstat_addr, (char*)&ps, sizeof(ps)) < 0)
    return -1;

  return 0;
}