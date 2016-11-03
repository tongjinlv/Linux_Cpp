obj-m := hello.o
KERNEL_DIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
all:
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules
	echo $(KERNEL_DIR)
clean:
	rm *.o *.ko *.mod.c

.PHONY:clean
