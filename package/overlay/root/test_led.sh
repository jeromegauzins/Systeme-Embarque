for i in `seq 1 10`;
do
	modprobe gpiooo
	sleep 1
	rmmod gpiooo
	sleep 1
done
