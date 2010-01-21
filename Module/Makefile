KERNEL_DIR ?= /lib/modules/`uname -r`/build

obj-m = main.o
main-objs := main.o kbuf.o

all:
	@make -C $(KERNEL_DIR) M=`pwd` modules

clean:
	make -C $(KERNEL_DIR) M=`pwd` clean

