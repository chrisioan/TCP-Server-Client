#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

int main(){
  for (int i = 0; i < 20; i++){
  pid_t p = fork();
  if (!p){
    char* argv[] = { "remoteClient","-i","127.0.0.1","-p","12500","-d","Server", NULL};
  execv("./remoteClient",argv);
  }
}
while(wait(NULL) > 0 );
}
