/* CS3500: Operating Systems and Lab
** ---------------------------------
** Assignment - 1 (Part 1)
** 
** Author: Akilesh Kannan (EE18B122)
** File name: printArgs.c
** Description: Program to print all arguments
**              used in its invocation
*/

#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char* argv[]){

  for (int i=0;i<argc;i+=1)
    fprintf(2, "arg%d is %s\n", i, argv[i]);

  exit(0);
}
