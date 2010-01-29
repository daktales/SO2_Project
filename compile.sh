#!bin/sh
cd Module/
make -C ../Vm/linux-2.6.19 M=`pwd` modules
cd ../Writer/
make
cd ../Reader/
make
cd ..

