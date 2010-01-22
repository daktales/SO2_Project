KERNEL_DIR ?= /lib/modules/`uname -r`/build

obj-m = mydev.o
mydev-objs := main.o kbuf.o

all:
	@make -C $(KERNEL_DIR) M=`pwd` modules

clean:
	rm  *.o *.mod.* Module.symvers .mydev.*
	rm -r .*
