obj-m := toshibaUsbInterface.o

KDIR := /lib/modules/$(shell uname -r)/build

all:
	make build
	make run
build:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
run:
	sudo insmod toshibaUsbInterface.ko
	sudo rmmod toshibaUsbInterface.ko
	sudo dmesg | tail
