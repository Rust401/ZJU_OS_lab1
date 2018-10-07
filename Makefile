TARGET=process_module
KDIR=/usr/src/linux
PWD=$(shell pwd)
obj-m += $(TARGET).o
default:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
