# stackdev
Simple Linux kernel module

Simulates the character device, which has a stack-organized memory.

Stack can hold up to 16 INT values 

Return codes:  
-ENOMEM when full   
-EINVAL when empty  
-ENOSYS when read/write size is not a multiple of INT size (4)  
  
stackdev.c - source code  
test_read_write.c - test program


