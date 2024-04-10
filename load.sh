insmod toshibaUsbInterface.ko
sleep 10
rmmod toshibaUsbInterface.ko
dmesg | tail -n 20 
