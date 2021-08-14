/* CS3500: Operating Systems and Lab
** ---------------------------------
** Assignment - 1 (Part 2)
** 
** Author: Akilesh Kannan (EE18B122)
** File name: sleep.c
** Description: User-mode program to sleep 
*/

#include "kernel/types.h"
#include "kernel/syscall.h"
#include "user/user.h"

int main(int argc, char* argv[]){
  
  int ticks;
  if(argc != 2){
    printf("usage: sleep <n>\n");
    exit(1);
  }

  printf("[USER]: entered user/sleep.c\n");
  ticks = atoi(argv[1]);
  sleep(ticks);
  printf("[USER]: exited user/sleep.c\n");
  exit(0);
}
