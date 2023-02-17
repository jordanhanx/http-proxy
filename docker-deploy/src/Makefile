# 
PROGS = http-proxy
SRCS = $(wildcard *.cpp)
PROG_OBJS = $(patsubst %, %.o, $(PROGS))
OBJS = $(filter-out $(PROG_OBJS), $(patsubst %.cpp, %.o, $(SRCS)))

# compile option
CC = g++
CFLAGS = -std=c++11 -Wall -Werror -pedantic -ggdb3
LIB = -lpthread

# task/target
all: $(PROGS)

http-proxy: http-proxy.o $(OBJS)
	$(CC) -o $@ $(CFLAGS) $^ $(LIB)

%.o: %.cpp
	$(CC) -c $(CFLAGS) $<


.PHNOY: clean depend
clean:
	rm -f *~ *.o $(PROGS)
depend:
	makedepend $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.