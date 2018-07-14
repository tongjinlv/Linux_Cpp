g++ -c tcp_client.cpp -o tcp_client.o
g++ -g tcp_client.o main.cpp -I. -o datamark
./datamark 60.205.30.37 80 565444444444444
