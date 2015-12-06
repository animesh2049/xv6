8350 # Initial process execs /init.
8351 
8352 #include "syscall.h"
8353 #include "traps.h"
8354 
8355 
8356 # exec(init, argv)
8357 .globl start
8358 start:
8359   pushl $argv
8360   pushl $init
8361   pushl $0  // where caller pc would be
8362   movl $SYS_exec, %eax
8363   int $T_SYSCALL
8364 
8365 # for(;;) exit();
8366 exit:
8367   movl $SYS_exit, %eax
8368   int $T_SYSCALL
8369   jmp exit
8370 
8371 # char init[] = "/init\0";
8372 init:
8373   .string "/init\0"
8374 
8375 # char *argv[] = { init, 0 };
8376 .p2align 2
8377 argv:
8378   .long init
8379   .long 0
8380 
8381 
8382 
8383 
8384 
8385 
8386 
8387 
8388 
8389 
8390 
8391 
8392 
8393 
8394 
8395 
8396 
8397 
8398 
8399 
