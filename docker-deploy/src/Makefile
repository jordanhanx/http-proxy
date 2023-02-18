### compile option
CC = g++
CFLAGS = -std=c++11 -Wall -Werror -pedantic -ggdb3
LIB = -lpthread

### variables
PROGS = http-caching-proxy
MAIN_OBJS = $(patsubst %, %.o, $(PROGS))
SRCS = $(wildcard *.cpp) $(wildcard */*.cpp)
OBJS = $(filter-out $(MAIN_OBJS), $(patsubst %.cpp, %.o, $(notdir $(SRCS))))

VPATH = socket proxy-server # search subdirectories
# $(info    PROGS is $(PROGS)) # print variable
# $(info    OBJS is $(OBJS)) # print variable

### task/target
all: $(PROGS)

$(PROGS): %: %.o $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIB)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<


.PHNOY: clean depend
clean:
	rm -f *~ *.o $(PROGS)
# depend:
# 	makedepend $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.