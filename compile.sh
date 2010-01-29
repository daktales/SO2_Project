#!bin/sh
cd Module/
make -C ../Vm/linux-2.6.19 M=`pwd` modules
cd -

