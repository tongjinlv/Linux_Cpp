!#/bin/bash
for((i=0;i<100;i++))
do 
python cap.py
mv raw.jpg 4UTmPcpw.jpg
python post.py 4UTmPcpw.jpg
done
