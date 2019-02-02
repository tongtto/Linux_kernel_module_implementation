#include <linux/module.h>
#include <linux/kernel.h>
MODULE_LICENSE("ROOTKIT");
MODULE_AUTHOR("TONG");
char* pid0="";
module_param(pid0,charp,0000);
MODULE_PARM_DESC(pid0, "A string");
static int initialize_sneaky_module(void){
  
 printk(KERN_INFO "My pid is : %s", pid0);
 return 0;
}
static void exit_sneaky_module(void){
  printk(KERN_INFO "Sneaky module being unloaded.\n");
}
module_init(initialize_sneaky_module);  // what's called upon loading
module_exit(exit_sneaky_module);        // what's called upon unloading
