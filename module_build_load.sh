make clean
make

if lsmod | grep "stackdev" &>2; then
	sudo rmmod stackdev
fi
sudo insmod stackdev.ko