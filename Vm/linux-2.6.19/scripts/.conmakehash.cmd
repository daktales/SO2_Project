cmd_scripts/conmakehash := gcc -Wp,-MD,scripts/.conmakehash.d -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer     -o scripts/conmakehash scripts/conmakehash.c  

deps_scripts/conmakehash := \
  scripts/conmakehash.c \
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
  /usr/include/sysexits.h \
  /usr/include/string.h \
  /usr/include/bits/string.h \
  /usr/include/x86_64-linux/i386-linux/bits/string.h \
  /usr/include/bits/string2.h \
  /usr/include/ctype.h \

scripts/conmakehash: $(deps_scripts/conmakehash)

$(deps_scripts/conmakehash):
