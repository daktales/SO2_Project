cmd_scripts/kconfig/kxgettext.o := gcc -Wp,-MD,scripts/kconfig/.kxgettext.o.d -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer   -DCURSES_LOC="<ncurses.h>" -DLOCALE  -c -o scripts/kconfig/kxgettext.o scripts/kconfig/kxgettext.c

deps_scripts/kconfig/kxgettext.o := \
  scripts/kconfig/kxgettext.c \
  /usr/include/stdlib.h \
  /usr/include/features.h \
  /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/include/x86_64-linux/i386-linux/gnu/stubs.h \
  /usr/lib/gcc-lib/i486-linux/3.3.5/include/stddef.h \
  /usr/include/sys/types.h \
  /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h \
  /usr/include/x86_64-linux/i386-linux/bits/wordsize.h \
  /usr/include/bits/typesizes.h \
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
  scripts/kconfig/lkc.h \
    $(wildcard include/config/list.h) \
  scripts/kconfig/expr.h \
  /usr/include/stdio.h \
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
  /usr/lib/gcc-lib/i486-linux/3.3.5/include/stdbool.h \
  /usr/include/libintl.h \
  /usr/include/locale.h \
  /usr/include/bits/locale.h \
  scripts/kconfig/lkc_proto.h \

scripts/kconfig/kxgettext.o: $(deps_scripts/kconfig/kxgettext.o)

$(deps_scripts/kconfig/kxgettext.o):
