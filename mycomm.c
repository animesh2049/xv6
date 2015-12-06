#include "types.h"
#include "stat.h"
#include "user.h"


typedef int (*FunctionCallback)(char *);

int 
main(int argc, char *argv[])
{
  int pid;
  int a,b;
  FunctionCallback functions[] = {&mkdir, &mkdir};

  int num=1;
  if(argc < 2){
	printf(2, "Usage: wait command ...\n");
	exit();
  }
  if(argv[1][0] == 'l' && argv[1][1] == 's'){
	num = 0;
  }
  else if(argv[1][0] == 'm'){
	num = 1;
  }
  else {
	printf(1,"%d",num);
	printf(2, "command not supported\n");
	exit();
  }
  //int mypid = getpid();
  //printf(1, "my pid from start is:%d\n", mypid);
  pid = fork();
  if(pid < 0)
  {
	printf(2, "Fork failed\n");
	exit();
  }
  functions[num](argv[2]);
  if(pid != 0){
  if(waitx(&a, &b) < 0){
	printf(2, "waitx: %s failed to execute\n", argv[1]);
  }
  printf(1, "rumtime is:%d wait time is:%d\n", a, b);
  }
  exit();
}
