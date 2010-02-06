#!bin/sh
QEMU="qemu-kvm"
which "${QEMU}" &>/dev/null || QEMU="qemu"

pkill qemu
cp Module/mydev.ko Vm/ramfs/
cp Writer/writer Vm/ramfs/
cp Reader/reader Vm/ramfs/
cd Vm/ramfs
sh mkinitramfs.sh ../tmpfs
cd -
${QEMU} -kernel Vm/linux-2.6.19/bzImage -initrd Vm/tmpfs -hda /dev/null -m 128&

