#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void) {
  fork();
  wait(0);
  exit(0);
}
