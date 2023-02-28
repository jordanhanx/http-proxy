#!/bin/bash
make clean

make

while true
do 
	./cpp_build/http-caching-proxy 12345 4
	sleep 1 
done