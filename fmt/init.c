8450 // init: The initial user-level program
8451 
8452 #include "types.h"
8453 #include "stat.h"
8454 #include "user.h"
8455 #include "fcntl.h"
8456 
8457 char *argv[] = { "sh", 0 };
8458 
8459 int
8460 main(void)
8461 {
8462   int pid, wpid;
8463 
8464   if(open("console", O_RDWR) < 0){
8465     mknod("console", 1, 1);
8466     open("console", O_RDWR);
8467   }
8468   dup(0);  // stdout
8469   dup(0);  // stderr
8470 
8471   for(;;){
8472     printf(1, "init: starting sh\n");
8473     pid = fork();
8474     if(pid < 0){
8475       printf(1, "init: fork failed\n");
8476       exit();
8477     }
8478     if(pid == 0){
8479       exec("sh", argv);
8480       printf(1, "init: exec sh failed\n");
8481       exit();
8482     }
8483     while((wpid=wait()) >= 0 && wpid != pid)
8484       printf(1, "zombie!\n");
8485   }
8486 }
8487 
8488 
8489 
8490 
8491 
8492 
8493 
8494 
8495 
8496 
8497 
8498 
8499 
