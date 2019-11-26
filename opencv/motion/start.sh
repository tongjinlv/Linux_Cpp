#!/bin/sh
while true
do
count=`ps aux|grep python|wc -l`
if [ $count -ge 2 ]
then
		sleep 5
else
        echo "start" 
		python /home/pi/work/Linux_Cpp/opencv/motion/bt.py &
		sleep 5
fi
done
