ifneq ($(KERNELRELEASE),)  
  
obj-m := hello.o  
  
else  
      
KDIR := /home/hcx/work/boards/RPi/kernel/linux-rpi-3.6.y  
all:  
	make -C $(KDIR) M=$(PWD) modules ARCH=arm CROSS_COMPILE=/home/hcx/work/boards/RPi/kernel/RpiTools/arm-bcm2708/arm-bcm2708hardfp-linux-gnueabi/bin/arm-bcm2708hardfp-linux-gnueabi-  
  
clean:  
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers  modul*  
  
endif 
