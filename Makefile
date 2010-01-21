KERNEL_DIR ?= /usr/src/linux-`uname -r`

obj-m = test.o

all:
	make -C $(KERNEL_DIR) M=`pwd` modules

clean:
	make -C $(KERNEL_DIR) M=`pwd` clean

