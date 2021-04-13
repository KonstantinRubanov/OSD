for host in /sys/bus/usb/devices/usb*; do echo 0 > $host/authorized_default; done
