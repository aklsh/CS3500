#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char* argv[]){
  if(argc < 2)
    printf("Usage: test_program_2 [arg1] [arg2]...\n");
  else
    echo_kernel(argc, argv);
  exit(0);
}
