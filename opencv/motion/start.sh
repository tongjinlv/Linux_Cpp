#!/bin/sh
while true
do
count=`ps aux|grep python|wc -l`
if [ $count -ge 2 ]
then
		sleep 5
else
        echo "start" 
	        sudo mkdir /mnt/udisk
                sudo mount -o uid=pi,gid=pi /dev/sda1 /mnt/udisk
		python /home/pi/work/Linux_Cpp/opencv/motion/mt.py &
		sleep 5
fi
done
