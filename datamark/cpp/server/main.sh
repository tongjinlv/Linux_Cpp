g++ -c tcp_server.cpp -o tcp_server.o
g++ -g tcp_server.o main.cpp -I. -o main
./main 8081
