cmd_scripts/mod/file2alias.o := gcc -Wp,-MD,scripts/mod/.file2alias.o.d -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer     -c -o scripts/mod/file2alias.o scripts/mod/file2alias.c

deps_scripts/mod/file2alias.o := \
  scripts/mod/file2alias.c \
  scripts/mod/modpost.h \
  /usr/include/stdio.h \
  /usr/include/features.h \
  /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/include/x86_64-linux/i386-linux/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.5/include/stddef.h \
  /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h \
  /usr/include/x86_64-linux/i386-linux/bits/wordsize.h \
  /usr/include/bits/typesizes.h \
  /usr/include/libio.h \
  /usr/include/_G_config.h \
  /usr/include/wchar.h \
  /usr/include/bits/wchar.h \
  /usr/include/x86_64-linux/i386-linux/bits/wchar.h \
  /usr/include/gconv.h \
  /usr/lib/gcc-lib/i486-linux/3.3.5/include/stdarg.h \
  /usr/include/bits/stdio_lim.h \
  /usr/include/bits/sys_errlist.h \
  /usr/include/bits/stdio.h \
  /usr/include/stdlib.h \
  /usr/include/sys/types.h \
  /usr/include/time.h \
  /usr/include/endian.h \
  /usr/include/bits/endian.h \
  /usr/include/x86_64-linux/i386-linux/bits/endian.h \
  /usr/include/sys/select.h \
  /usr/include/bits/select.h \
  /usr/include/x86_64-linux/i386-linux/bits/select.h \
  /usr/include/bits/sigset.h \
  /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h \
  /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h \
  /usr/include/alloca.h \
  /usr/include/string.h \
  /usr/include/bits/string.h \
  /usr/include/x86_64-linux/i386-linux/bits/string.h \
  /usr/include/bits/string2.h \
  /usr/include/sys/stat.h \
  /usr/include/bits/stat.h \
  /usr/include/x86_64-linux/i386-linux/bits/stat.h \
  /usr/include/sys/mman.h \
  /usr/include/bits/mman.h \
  /usr/include/x86_64-linux/i386-linux/bits/mman.h \
  /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h \
  /usr/include/x86_64-linux/i386-linux/bits/fcntl.h \
  /usr/include/unistd.h \
  /usr/include/bits/posix_opt.h \
  /usr/include/x86_64-linux/i386-linux/bits/posix_opt.h \
  /usr/include/bits/confname.h \
  /usr/include/getopt.h \
  /usr/include/elf.h \
  /usr/include/stdint.h \
  scripts/mod/elfconfig.h \
  /usr/include/ctype.h \
  include/linux/mod_devicetable.h \
  include/linux/input.h \
  /usr/include/sys/time.h \
  /usr/include/sys/ioctl.h \
  /usr/include/bits/ioctls.h \
  /usr/include/asm/ioctls.h \
  /usr/include/asm-i386/ioctls.h \
  /usr/include/asm/ioctl.h \
  /usr/include/asm-i386/ioctl.h \
  /usr/include/bits/ioctl-types.h \
  /usr/include/sys/ttydefaults.h \
  /usr/include/asm/types.h \
  /usr/include/asm-i386/types.h \
    $(wildcard include/config/highmem64g.h) \
    $(wildcard include/config/lbd.h) \

scripts/mod/file2alias.o: $(deps_scripts/mod/file2alias.o)

$(deps_scripts/mod/file2alias.o):
