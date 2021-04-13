for dev in $(sudo cat /etc/fstab | grep "^.\dev/sr[0-9].*" | sudo awk '{print $2}'); do  if [ -e "$dev" ]; then   sudo umount $dev 2>/dev/null; fi; done;

