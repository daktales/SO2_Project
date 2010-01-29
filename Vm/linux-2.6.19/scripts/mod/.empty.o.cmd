cmd_scripts/mod/empty.o := gcc -m32 -Wp,-MD,scripts/mod/.empty.o.d  -nostdinc -isystem /usr/lib/gcc-lib/i486-linux/3.3.5/include -D__KERNEL__ -Iinclude  -include include/linux/autoconf.h -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -O2 -pipe -msoft-float -mpreferred-stack-boundary=2  -march=i686 -mcpu=pentium4  -ffreestanding -maccumulate-outgoing-args -DCONFIG_AS_CFI=1  -Iinclude/asm-i386/mach-default -fomit-frame-pointer       -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(empty)"  -D"KBUILD_MODNAME=KBUILD_STR(empty)" -c -o scripts/mod/empty.o scripts/mod/empty.c

deps_scripts/mod/empty.o := \
  scripts/mod/empty.c \

scripts/mod/empty.o: $(deps_scripts/mod/empty.o)

$(deps_scripts/mod/empty.o):
