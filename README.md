## install package

sudo apt install libssl-dev

## compile server
g++ -I /usr/include/boost -pthread example.cpp 
g++ -I /usr/include/boost -pthread example.cpp -o example

## run server
./example 0.0.0.0 8080 .

## compile client 
same

## run client 
 ./client www.example.com 80 /
  ./client www.example.com 80 / 1.0