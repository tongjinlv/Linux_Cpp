while [ true ]; do
 ccc=$(last -n 1)
 ccc=$(df -hl |grep /dev/vda1)
 echo $ccc
 ./client/datamark 60.205.30.37 80 "$ccc"
 sleep 10s
done
