8400 #include "syscall.h"
8401 #include "traps.h"
8402 
8403 #define SYSCALL(name) \
8404   .globl name; \
8405   name: \
8406     movl $SYS_ ## name, %eax; \
8407     int $T_SYSCALL; \
8408     ret
8409 
8410 SYSCALL(fork)
8411 SYSCALL(exit)
8412 SYSCALL(wait)
8413 SYSCALL(pipe)
8414 SYSCALL(read)
8415 SYSCALL(write)
8416 SYSCALL(close)
8417 SYSCALL(kill)
8418 SYSCALL(exec)
8419 SYSCALL(open)
8420 SYSCALL(mknod)
8421 SYSCALL(unlink)
8422 SYSCALL(fstat)
8423 SYSCALL(link)
8424 SYSCALL(mkdir)
8425 SYSCALL(chdir)
8426 SYSCALL(dup)
8427 SYSCALL(getpid)
8428 SYSCALL(sbrk)
8429 SYSCALL(sleep)
8430 SYSCALL(uptime)
8431 SYSCALL(waitx)
8432 
8433 
8434 
8435 
8436 
8437 
8438 
8439 
8440 
8441 
8442 
8443 
8444 
8445 
8446 
8447 
8448 
8449 
