## install package

sudo apt install libssl-dev

## compile server
g++ -I /usr/include/boost -pthread example.cpp 
g++ -I /usr/include/boost -pthread example.cpp -o example


## run server
```
./example 0.0.0.0 8080 .
```


## compile client 
g++ -I /usr/include/boost -pthread client.cpp -o client

## run client 
need to type 4 input: method(get/post), host, port & target
```
./client get www.example.com 80 /
./client post www.example.com 80 / 1.0
```

more example to try:
```
./client post www.rssweather.com 80 /
 ./client get www.gnu.org 80 /
```

current version not handle any exception yet
e.g 502 and 400

