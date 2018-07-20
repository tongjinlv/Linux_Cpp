g++ -c tcp_client.cpp -o tcp_client.o
g++ -g tcp_client.o main.cpp -I. -o datamark
./datamark trtos.com 80 1fDkeEG3 "wo ai yu lan"
