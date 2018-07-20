while [ true ]; do
 ccc=$(free |grep Mem)
 echo $ccc
 ./client/datamark 60.205.30.37 80 "$ccc"
 sleep 10s
done
