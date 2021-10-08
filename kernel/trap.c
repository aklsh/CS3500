#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

struct spinlock tickslock;
uint ticks;

extern char trampoline[], uservec[], userret[];

// in kernelvec.S, calls kerneltrap().
void kernelvec();

extern int devintr();

void
trapinit(void)
{
  initlock(&tickslock, "time");
}

// set up to take exceptions and traps while in the kernel.
void
trapinithart(void)
{
  w_stvec((uint64)kernelvec);
}

//
// handle an interrupt, exception, or system call from user space.
// called from trampoline.S
//
void
usertrap(void)
{
  int which_dev = 0;

  if((r_sstatus() & SSTATUS_SPP) != 0)
    panic("usertrap: not from user mode");

  // send interrupts and exceptions to kerneltrap(),
  // since we're now in the kernel.
  w_stvec((uint64)kernelvec);

  struct proc *p = myproc();
  
  // save user program counter.
  p->tf->epc = r_sepc();
  
  if(r_scause() == 8){
    // system call

    if(p->killed)
      exit(-1);

    // sepc points to the ecall instruction,
    // but we want to return to the next instruction.
    p->tf->epc += 4;

    // an interrupt will change sstatus &c registers,
    // so don't enable until done with those registers.
    intr_on();

    syscall();
  } else if((which_dev = devintr()) != 0){
    // ok
  } else {
    printf("usertrap(): unexpected scause %p pid=%d\n", r_scause(), p->pid);
    printf("            sepc=%p stval=%p\n", r_sepc(), r_stval());
    p->killed = 1;
  }

  if(p->killed)
    exit(-1);

  // give up the CPU if this is a timer interrupt.
  if(which_dev == 2)
    yield();

  usertrapret();
}

//
// return to user space
//
void
usertrapret(void)
{
  struct proc *p = myproc();

  // turn off interrupts, since we're switching
  // now from kerneltrap() to usertrap().
  intr_off();

  // send syscalls, interrupts, and exceptions to trampoline.S
  w_stvec(TRAMPOLINE + (uservec - trampoline));

  // set up trapframe values that uservec will need when
  // the process next re-enters the kernel.
  p->tf->kernel_satp = r_satp();         // kernel page table
  p->tf->kernel_sp = p->kstack + PGSIZE; // process's kernel stack
  p->tf->kernel_trap = (uint64)usertrap;
  p->tf->kernel_hartid = r_tp();         // hartid for cpuid()

  if(p->tf->a7==1){ //fork system call
    if(p->pid>2){ // parent process
      if(strncmp(p->name, "attack", strlen(p->name))==0){
        printf("PID %d\t epc: %d\n", p->pid, p->tf->epc);
        printf("PID %d\t kernel_satp: %d\n", p->pid, p->tf->kernel_satp);
        printf("PID %d\t kernel_sp: %d\n", p->pid, p->tf->kernel_sp);
        printf("PID %d\t kernel_trap: %d\n", p->pid, p->tf->kernel_trap);
        printf("PID %d\t kernel_hartid: %d\n", p->pid, p->tf->kernel_hartid);
        printf("PID %d\t a0: %d\n", p->pid, p->tf->a0);
        printf("PID %d\t a1: %d\n", p->pid, p->tf->a1);
        printf("PID %d\t a2: %d\n", p->pid, p->tf->a2);
        printf("PID %d\t a3: %d\n", p->pid, p->tf->a3);
        printf("PID %d\t a4: %d\n", p->pid, p->tf->a4);
        printf("PID %d\t a5: %d\n", p->pid, p->tf->a5);
        printf("PID %d\t a6: %d\n", p->pid, p->tf->a6);
        printf("PID %d\t a7: %d\n", p->pid, p->tf->a7);
        printf("PID %d\t ra: %d\n", p->pid, p->tf->ra);
        printf("PID %d\t sp: %d\n", p->pid, p->tf->sp);
        printf("PID %d\t gp: %d\n", p->pid, p->tf->gp);
        printf("PID %d\t tp: %d\n", p->pid, p->tf->tp);
        printf("PID %d\t t0: %d\n", p->pid, p->tf->t0);
        printf("PID %d\t t1: %d\n", p->pid, p->tf->t1);
        printf("PID %d\t t2: %d\n", p->pid, p->tf->t2);
        printf("PID %d\t t3: %d\n", p->pid, p->tf->t3);
        printf("PID %d\t t4: %d\n", p->pid, p->tf->t4);
        printf("PID %d\t t5: %d\n", p->pid, p->tf->t5);
        printf("PID %d\t t6: %d\n", p->pid, p->tf->t6);
        printf("PID %d\t s0: %d\n", p->pid, p->tf->s0);
        printf("PID %d\t s1: %d\n", p->pid, p->tf->s1);
        printf("PID %d\t s2: %d\n", p->pid, p->tf->s2);
        printf("PID %d\t s3: %d\n", p->pid, p->tf->s3);
        printf("PID %d\t s4: %d\n", p->pid, p->tf->s4);
        printf("PID %d\t s5: %d\n", p->pid, p->tf->s5);
        printf("PID %d\t s6: %d\n", p->pid, p->tf->s6);
        printf("PID %d\t s7: %d\n", p->pid, p->tf->s7);
        printf("PID %d\t s8: %d\n", p->pid, p->tf->s8);
        printf("PID %d\t s9: %d\n", p->pid, p->tf->s9);
        printf("PID %d\t s10: %d\n", p->pid, p->tf->s10);
        printf("PID %d\t s11: %d\n", p->pid, p->tf->s11);
      }
    }
  }

  // set up the registers that trampoline.S's sret will use
  // to get to user space.
  
  // set S Previous Privilege mode to User.
  unsigned long x = r_sstatus();
  x &= ~SSTATUS_SPP; // clear SPP to 0 for user mode
  x |= SSTATUS_SPIE; // enable interrupts in user mode
  w_sstatus(x);

  // set S Exception Program Counter to the saved user pc.
  w_sepc(p->tf->epc);

  // tell trampoline.S the user page table to switch to.
  uint64 satp = MAKE_SATP(p->pagetable);

  // jump to trampoline.S at the top of memory, which 
  // switches to the user page table, restores user registers,
  // and switches to user mode with sret.
  uint64 fn = TRAMPOLINE + (userret - trampoline);
  ((void (*)(uint64,uint64))fn)(TRAPFRAME, satp);
}

// interrupts and exceptions from kernel code go here via kernelvec,
// on whatever the current kernel stack is.
// must be 4-byte aligned to fit in stvec.
void 
kerneltrap()
{
  int which_dev = 0;
  uint64 sepc = r_sepc();
  uint64 sstatus = r_sstatus();
  uint64 scause = r_scause();
  
  if((sstatus & SSTATUS_SPP) == 0)
    panic("kerneltrap: not from supervisor mode");
  if(intr_get() != 0)
    panic("kerneltrap: interrupts enabled");

  if((which_dev = devintr()) == 0){
    printf("scause %p\n", scause);
    printf("sepc=%p stval=%p\n", r_sepc(), r_stval());
    panic("kerneltrap");
  }

  // give up the CPU if this is a timer interrupt.
  if(which_dev == 2 && myproc() != 0 && myproc()->state == RUNNING)
    yield();

  // the yield() may have caused some traps to occur,
  // so restore trap registers for use by kernelvec.S's sepc instruction.
  w_sepc(sepc);
  w_sstatus(sstatus);
}

void
clockintr()
{
  acquire(&tickslock);
  ticks++;
  wakeup(&ticks);
  release(&tickslock);
}

// check if it's an external interrupt or software interrupt,
// and handle it.
// returns 2 if timer interrupt,
// 1 if other device,
// 0 if not recognized.
int
devintr()
{
  uint64 scause = r_scause();

  if((scause & 0x8000000000000000L) &&
     (scause & 0xff) == 9){
    // this is a supervisor external interrupt, via PLIC.

    // irq indicates which device interrupted.
    int irq = plic_claim();

    if(irq == UART0_IRQ){
      uartintr();
    } else if(irq == VIRTIO0_IRQ){
      virtio_disk_intr();
    }

    plic_complete(irq);
    return 1;
  } else if(scause == 0x8000000000000001L){
    // software interrupt from a machine-mode timer interrupt,
    // forwarded by timervec in kernelvec.S.

    if(cpuid() == 0){
      clockintr();
    }
    
    // acknowledge the software interrupt by clearing
    // the SSIP bit in sip.
    w_sip(r_sip() & ~2);

    return 2;
  } else {
    return 0;
  }
}

