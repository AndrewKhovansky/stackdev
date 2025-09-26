gcc -o stackdev_test test_read_write.c

#Write some values to the device
./stackdev_test W 12 8 333 14 -22 78643

#Try to read 16 values at once
./stackdev_test R 16