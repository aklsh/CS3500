#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/processinfo.h"
#include "user/user.h"

int main(int argc, char* argv[]){
  struct processinfo info;
  if(argc != 1){
    printf("Usage: test_program_5\n");
    exit(1);
  }
  else
    get_process_info(&info);
  printf("Process ID -> %d\n", info.pid);
  printf("Process Name -> %s\n", info.name);
  printf("Memory size -> %d Bytes\n", info.sz);
  exit(0);
}
