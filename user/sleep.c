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

  ticks = atoi(argv[1]);
  printf("[USER]: before sleep\n");
  sleep(ticks);
  printf("[USER]: after sleep\n");
  exit(0);
}
