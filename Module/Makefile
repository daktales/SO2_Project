obj-m = mydev.o
mydev-objs := main.o kbuf.o stat.o

KERNEL_HEADER_PATH = ../Vm/linux-2.6.19

all:
	@make -C $(KERNEL_HEADER_PATH) M=`pwd` ARCH=i386 modules

clean:
	make -C $(KERNEL_HEADER_PATH) M=`pwd` ARCH=i386 clean
	@rm -rf *.o *~ core .*.cmd	*.ko *.mod.c .tmp_version .tmp_version test *.order *.symvers

