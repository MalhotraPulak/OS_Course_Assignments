cmake_minimum_required(VERSION 3.17)
project(Assignment5 C)

set(CMAKE_C_STANDARD 11)

include_directories(2019101050-xv6/xv6)
include_directories(2019101050-xv6/xv6-public)

add_executable(Assignment5
        2019101050-xv6/xv6/asm.h
        2019101050-xv6/xv6/bio.c
        2019101050-xv6/xv6/bootmain.c
        2019101050-xv6/xv6/buf.h
        2019101050-xv6/xv6/cat.c
        2019101050-xv6/xv6/console.c
        2019101050-xv6/xv6/date.h
        2019101050-xv6/xv6/defs.h
        2019101050-xv6/xv6/echo.c
        2019101050-xv6/xv6/elf.h
        2019101050-xv6/xv6/exec.c
        2019101050-xv6/xv6/fcntl.h
        2019101050-xv6/xv6/file.c
        2019101050-xv6/xv6/file.h
        2019101050-xv6/xv6/forktest.c
        2019101050-xv6/xv6/fs.c
        2019101050-xv6/xv6/fs.h
        2019101050-xv6/xv6/grep.c
        2019101050-xv6/xv6/ide.c
        2019101050-xv6/xv6/init.c
        2019101050-xv6/xv6/ioapic.c
        2019101050-xv6/xv6/kalloc.c
        2019101050-xv6/xv6/kbd.c
        2019101050-xv6/xv6/kbd.h
        2019101050-xv6/xv6/kill.c
        2019101050-xv6/xv6/lapic.c
        2019101050-xv6/xv6/ln.c
        2019101050-xv6/xv6/log.c
        2019101050-xv6/xv6/ls.c
        2019101050-xv6/xv6/main.c
        2019101050-xv6/xv6/memide.c
        2019101050-xv6/xv6/memlayout.h
        2019101050-xv6/xv6/mkdir.c
        2019101050-xv6/xv6/mkfs.c
        2019101050-xv6/xv6/mmu.h
        2019101050-xv6/xv6/mp.c
        2019101050-xv6/xv6/mp.h
        2019101050-xv6/xv6/param.h
        2019101050-xv6/xv6/picirq.c
        2019101050-xv6/xv6/pipe.c
        2019101050-xv6/xv6/printf.c
        2019101050-xv6/xv6/proc.c
        2019101050-xv6/xv6/proc.h
        2019101050-xv6/xv6/rm.c
        2019101050-xv6/xv6/sh.c
        2019101050-xv6/xv6/sleeplock.c
        2019101050-xv6/xv6/sleeplock.h
        2019101050-xv6/xv6/spinlock.c
        2019101050-xv6/xv6/spinlock.h
        2019101050-xv6/xv6/stat.h
        2019101050-xv6/xv6/stressfs.c
        2019101050-xv6/xv6/string.c
        2019101050-xv6/xv6/syscall.c
        2019101050-xv6/xv6/syscall.h
        2019101050-xv6/xv6/sysfile.c
        2019101050-xv6/xv6/sysproc.c
        2019101050-xv6/xv6/trap.c
        2019101050-xv6/xv6/traps.h
        2019101050-xv6/xv6/types.h
        2019101050-xv6/xv6/uart.c
        2019101050-xv6/xv6/ulib.c
        2019101050-xv6/xv6/umalloc.c
        2019101050-xv6/xv6/user.h
        2019101050-xv6/xv6/usertests.c
        2019101050-xv6/xv6/vm.c
        2019101050-xv6/xv6/wc.c
        2019101050-xv6/xv6/x86.h
        2019101050-xv6/xv6/zombie.c
        2019101050-xv6/xv6/time.c)


add_executable(rrScheduler 2019101050-xv6/xv6/RR.c)
add_executable(FCFS 2019101050-xv6/xv6/FCFS.c)
add_executable(benchmark 2019101050-xv6/xv6/benchmark.c)
add_executable(sysPriority 2019101050-xv6/xv6/setPriority.c)
add_executable(PBS 2019101050-xv6/xv6/PBS.c)
add_executable(ps 2019101050-xv6/xv6/ps.c)
add_executable(gm 2019101050-xv6/xv6/gm.c)

add_executable(MLFQ 2019101050-xv6/xv6/MLFQ.c)