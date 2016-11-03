#!/bin/sh
g++ -D_DEBUG -D_GNU_SOURCE -w -pthread -pipe -ldl -lrt -Iinclude -fPIC -o ISR ISR.cpp msc.so

