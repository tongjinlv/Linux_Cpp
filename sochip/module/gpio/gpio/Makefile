obj-m :=app.o
all:
	arm-linux-gnueabi-gcc -static app.c -o app
clean:
	rm -rf *.ko *.o *.order *symvers
