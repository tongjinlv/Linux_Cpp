g++ -c tcp_client.cpp -o tcp_client.o
g++ -g tcp_client.o main.cpp -I. -o datamark
./datamark trtos.com 80 5jH6NKUy "wo ai yu lan"
