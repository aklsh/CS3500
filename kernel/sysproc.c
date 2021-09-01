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
  exit(n);
  return 0;  // not reached
}

uint64 sys_getpid(void){
  return myproc()->pid;
}

uint64 sys_fork(void){
  return fork();
}

uint64 sys_wait(void){
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64 sys_sbrk(void){
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
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
  return 0;
}

uint64 sys_kill(void){
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64 sys_uptime(void){
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64 sys_echo_simple(void){
  char argument[MAXARGLENGTH];              // argument can have max length of 128 char
  if(argstr(0, argument, MAXARGLENGTH) < 0) // return error if no argument
    return -1;
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
  p->trace_mask = (uint32) mask;  // assign process trace mask
  return 0;
}

uint64 sys_get_process_info(void){
  struct processinfo info;
  uint64 struct_pointer;
  struct proc *p = myproc();

  for(int i=0;i<16;i++)
    info.name[i] = p->name[i];
  info.pid = p->pid;
  info.sz = p->sz;
  
  if(argaddr(0, &struct_pointer) < 0)
    return -1;
  if(copyout(p->pagetable, struct_pointer, (char*) &info, sizeof(info)) < 0)
    return -1;
  return 0;
}
