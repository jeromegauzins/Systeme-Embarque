maj = cat proc/devices | grep POULET
mknod POULET c $? 0
