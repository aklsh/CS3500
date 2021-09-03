#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "processinfo.h"

uint64 sys_exit(void){
  int n;
  if(argint(0, &n) < 0)
    return -1;
  if(myproc()->trace_mask & (1<<2))
    printf("Args: %d\n", 
            n);
  exit(n);
  return 0;  // not reached
}

uint64 sys_getpid(void){
  struct proc *p = myproc();
  if(p->trace_mask & (1<<11))
    printf("Args: <none>, ");
  return p->pid;
}

uint64 sys_fork(void){
  if(myproc()->trace_mask & (1<<1))
    printf("Args: <none>, ");
  return fork();
}

uint64 sys_wait(void){
  uint64 n;
  if(argaddr(0, &n) < 0)
    return -1;
  if(myproc()->trace_mask & (1<<3))
    printf("Args: %dv ", 
            n);
  return wait(n);
}

uint64 sys_sbrk(void){
  int addr;
  int n;
  struct proc *p = myproc();

  if(argint(0, &n) < 0)
    return -1;
  addr = p->sz;
  if(growproc(n) < 0)
    return -1;
  if(p->trace_mask & (1<<12))
    printf("Args: %d, ", 
            n);
  return addr;
}

uint64 sys_sleep(void){
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
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
  if(myproc()->trace_mask & (1<<13))
    printf("Args: %d, ", 
            n);
  return 0;
}

uint64 sys_kill(void){
  int pid;
  struct proc *p = myproc();

  if(argint(0, &pid) < 0)
    return -1;
  if(p->trace_mask & (1<<6))
    printf("Args: %d, ", 
            pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64 sys_uptime(void){
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  if(myproc()->trace_mask & (1<<14))
    printf("Args: <none>, ");
  return xticks;
}

uint64 sys_echo_simple(void){
  char argument[MAXARGLENGTH];              // argument can have max length of 128 char
  if(argstr(0, argument, MAXARGLENGTH) < 0) // return error if no argument
    return -1;
  if(myproc()->trace_mask & (1<<22))
    printf("Args: %s, ", 
            argument);
  printf("%s\n", argument);                 // print argument as string
  return 0;
}

uint64 sys_echo_kernel(void){
  int argc;
  char argument[MAXARGLENGTH+1];          // account for \0 in end.
  char* terminator;
  uint64 argv_base_addr;
  uint64 arg_addr;

  if((argint(0, &argc) < 0) || (argaddr(1, &argv_base_addr) < 0)) 
    return -1;                            // return error if no argument
  if(myproc()->trace_mask & (1<<23))
    printf("Args: %d,%p ", 
            argc, argv_base_addr);
  argv_base_addr += sizeof(char*);        // skip arg0 - name of program
  for(int i = 1; i < argc; i++){
    if((fetchaddr(argv_base_addr, &arg_addr) < 0) || (fetchstr(arg_addr, argument, MAXARGLENGTH) < 0))
      return -1;
    printf("%s", argument);               // print argument
    terminator = " ";
    if(i+1 == argc)                       // check whether to print <space> or <newline>
      terminator = "\n";
    printf("%s", terminator);             // print terminator
    argv_base_addr += sizeof(char*);      // go to next argument address
  }
  return 0;
}

uint64 sys_trace(void){
  int mask;                       // trace mask for process
  if(argint(0, &mask) < 0)
    return -1;                    // return if error
  struct proc *p = myproc();
  p->trace_mask = mask;           // assign process trace mask
  if(mask & (1<<24))
    printf("PID: %d, Syscall Name: trace, Args: %d, ", 
            p->pid, mask);
  return 0;
}

uint64 sys_get_process_info(void){
  struct processinfo info;
  uint64 destination;
  struct proc *p = myproc();

  for(int i=0;i<16;i++)
    info.name[i] = p->name[i];
  info.pid = p->pid;
  info.sz = p->sz;
  
  if(argaddr(0, &destination) < 0)
    return -1;
  if(copyout(p->pagetable, destination, (char*) &info, sizeof(info)) < 0)
    return -1;
  if(p->trace_mask & (1<<25))
    printf("Args: %p, ", 
            destination);
  return 0;
}
