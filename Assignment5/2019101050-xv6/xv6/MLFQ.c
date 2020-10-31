#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

#define WAIT_TIME 1
extern struct {
    struct spinlock lock;
    struct proc proc[NPROC];
} ptable;
// Interrupt descriptor table (shared by all CPUs).
extern struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers






extern int nextpid;

extern void forkret(void);

extern void trapret(void);


//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.



int prev_ticks = 0;

void
scheduler(void) {
    struct proc *p; // is the process to switch
    struct cpu *c = mycpu();
    c->proc = 0;
    for (;;) {
        // Enable interrupts on this processor.
        sti();

        // Loop over process table looking for process to run.
        acquire(&ptable.lock);
        //cprintf("HERE 1\n");
        for (int priorityLevel = 0; priorityLevel <= 4;) {
            //cprintf("Looking in Queue: %d\n", priorityLevel);
            int min_toe = 1e9;
            p = 0;
            for (struct proc *aProc = ptable.proc; aProc < &ptable.proc[NPROC]; aProc++) {
                if (aProc->state == RUNNABLE && aProc->cur_q == priorityLevel) {
                    //cprintf("Found one process in queue %d with pid %d\n", aProc->cur_q, aProc->pid);
                    if (aProc->toe < min_toe) {
                        min_toe = aProc->toe;
                        p = aProc;
                    }
                }
            }
            //cprintf("HERE 4\n");
            // if didnt find any process in the cur prio move to higher prio queue
            if (p == 0) {
                priorityLevel++;
                continue;
            }
            c->proc = p;

            switchuvm(p);
            p->ticks = (int) (1u << (unsigned int) priorityLevel); // assign ticks to process based on priority level
            p->state = RUNNING;
            p->n_run++;
            //cprintf("MLFQ: Process switching to %d in queue %d for %d ticks\n", p->pid, p->cur_q, p->ticks);
            swtch(&(c->scheduler), p->context);
            //cprintf("MLFQ: Back to scd!\n");
            switchkvm();
            // if the process uses entire time slice put move it down in prio
            if (p->ticks == 0) {
                if (p->state == ZOMBIE) {
                    p->cur_q = -1;
                } else if (p->cur_q != 4) {
                    p->cur_q++;
                    // cprintf("MLFQ: Process %d demoted to %d queue\n", p->pid, p->cur_q);
                }
            }
            //   p->toe = ticks;
            // aging
            // dont worry about it
            for (struct proc *aProc = ptable.proc; aProc < &ptable.proc[NPROC]; aProc++) {

                if (aProc->state == RUNNABLE) {
                    int max_wait_time = 1 << (priorityLevel + 6);
                    if (ticks - aProc->toe > max_wait_time) {
                        aProc->cur_q--;
                        if (aProc->cur_q < 0) {
                            aProc->cur_q = 0;
                        }
                        aProc->toe = ticks;
                        //cprintf("MLFQ: Process %d promoted to %d queue\n", aProc->pid, aProc->cur_q);
                    }
                }
            }


            // to get data for bonus
            /* if(ticks - prev_ticks > 100){
                 cprintf("%d, ", ticks);
                 for (struct proc *aProc = ptable.proc; aProc < &ptable.proc[NPROC]; aProc++) {
                     cprintf("%d, ", aProc->cur_q);
                 }
                 cprintf("\n");
                 prev_ticks = ticks;
             }*/
            // start from beginning
            priorityLevel = 0;
        }
        release(&ptable.lock);

    }
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf) {
    if (tf->trapno == T_SYSCALL) {
        if (myproc()->killed)
            exit();
        myproc()->tf = tf;
        syscall();
        if (myproc()->killed)
            exit();
        return;
    }

    switch (tf->trapno) {
        case T_IRQ0 + IRQ_TIMER:
            if (cpuid() == 0) {
                acquire(&tickslock);
                ticks++;
                // Updating the run time of the process running currently
                if (myproc() != 0 && myproc()->state == RUNNING) {
                    myproc()->rtime += 1;
                    myproc()->ticks -= 1; // reduce the assigned ticks to process by 1
                    if (myproc()->cur_q == 0) {
                        myproc()->q0++;
                    } else if (myproc()->cur_q == 1) {
                        myproc()->q1++;
                    } else if (myproc()->cur_q == 2) {
                        myproc()->q2++;
                    } else if (myproc()->cur_q == 3) {
                        myproc()->q3++;
                    } else if (myproc()->cur_q == 4) {
                        myproc()->q4++;
                    }
                } else if (myproc() != 0 && myproc()->state == SLEEPING) {
                    myproc()->iotime += 1;
                }

                wakeup(&ticks);
                release(&tickslock);
            }
            lapiceoi();
            break;
        case T_IRQ0 + IRQ_IDE:
            ideintr();
            lapiceoi();
            break;
        case T_IRQ0 + IRQ_IDE + 1:
            // Bochs generates spurious IDE1 interrupts.
            break;
        case T_IRQ0 + IRQ_KBD:
            kbdintr();
            lapiceoi();
            break;
        case T_IRQ0 + IRQ_COM1:
            uartintr();
            lapiceoi();
            break;
        case T_IRQ0 + 7:
        case T_IRQ0 + IRQ_SPURIOUS:
            cprintf("cpu%d: spurious interrupt at %x:%x\n",
                    cpuid(), tf->cs, tf->eip);
            lapiceoi();
            break;

            //PAGEBREAK: 13
        default:
            if (myproc() == 0 || (tf->cs & 3) == 0) {
                // In kernel, it must be our mistake.
                cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
                        tf->trapno, cpuid(), tf->eip, rcr2());
                panic("trap");
            }
            // In user space, assume process misbehaved.
            cprintf("pid %d %s: trap %d err %d on cpu %d "
                    "eip 0x%x addr 0x%x--kill proc\n",
                    myproc()->pid, myproc()->name, tf->trapno,
                    tf->err, cpuid(), tf->eip, rcr2());
            myproc()->killed = 1;
    }

    // Force process exit if it has been killed and is in user space.
    // (If it is still executing in the kernel, let it keep running
    // until it gets to the regular system call return.)
    if (myproc() && myproc()->killed && (tf->cs & 3) == DPL_USER)
        exit();

    // Force process to give up CPU on clock tick.
    // If interrupts were on while locks held, would need to check nlock.
    if (myproc() && myproc()->state == RUNNING &&
        tf->trapno == T_IRQ0 + IRQ_TIMER) {
        if (myproc()->ticks <= 0) // only yield when done with given ticks
            yield();
    }

    // Check if the process has been killed since we yielded
    if (myproc() && myproc()->killed && (tf->cs & 3) == DPL_USER)
        exit();
}

