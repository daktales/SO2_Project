KERNEL_DIR ?= /lib/modules/`uname -r`/build

obj-m = mydev.o
mydev-objs := main.o kbuf.o stat.o

all:
	@make -C $(KERNEL_DIR) M=`pwd` modules

clean:
	rm -r *.ko *.o *.mod.* Module.symvers .mydev.* .main.* .kbuf.* .stat.* .tmp_version/ 
