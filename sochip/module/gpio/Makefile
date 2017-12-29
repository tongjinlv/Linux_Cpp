obj-m :=led.o
KDIR :=/home/tom/work/sochip/linux-3.4
all:
	make -C $(KDIR) M=$(PWD) modules CROSS_COMPILE=arm-linux-gnueabi- ARCH=arm
clean:
	rm -rf *.ko *.o *.order *symvers
